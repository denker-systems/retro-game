/*
 * mixer.c - Software Audio Mixer för DOS
 * 
 * SYFTE:
 *   Mixar musik och ljudeffekter till en enda audio-stream.
 *   Använder double-buffering och IRQ för sömlös uppspelning.
 *   
 * ARKITEKTUR:
 *   1. Två DMA-buffertar (A och B) som växlar
 *   2. IRQ triggas när en buffert är klar
 *   3. Mixer fyller nästa buffert med musik + SFX
 *   4. Auto-init DMA loopar mellan buffertarna
 *   
 * MIXNING:
 *   8-bit unsigned PCM: mittenvärde = 128
 *   mix = ((musik - 128) + (sfx - 128)) / 2 + 128
 */

#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "mixer.h"

/* ========== KONSTANTER ========== */

#define MIXER_BUFFER_SIZE  2048    /* Bytes per buffert (~46ms latens @ 11025Hz) */
#define MIXER_SAMPLE_RATE  11025  /* Hz */
#define MIXER_NUM_CHANNELS 4      /* Max samtidiga ljud */

/* Sound Blaster portar */
#define SB_BASE     0x220
#define SB_RESET    (SB_BASE + 0x06)
#define SB_READ     (SB_BASE + 0x0A)
#define SB_WRITE    (SB_BASE + 0x0C)
#define SB_STATUS   (SB_BASE + 0x0E)

/* DMA portar för kanal 1 */
#define DMA_MASK    0x0A
#define DMA_MODE    0x0B
#define DMA_FLIP    0x0C
#define DMA_ADDR    0x02
#define DMA_COUNT   0x03
#define DMA_PAGE    0x83

/* PIC portar */
#define PIC_CMD     0x20
#define PIC_MASK    0x21
#define PIC_EOI     0x20

/* ========== GLOBALA VARIABLER ========== */

/* Double-buffer */
static unsigned char far *buffer_a = NULL;
static unsigned char far *buffer_b = NULL;
static unsigned char far *current_play = NULL;  /* Buffer som spelas */
static unsigned char far *current_fill = NULL;  /* Buffer som fylls */

/* Mixer-kanaler */
typedef struct {
    unsigned char far *data;    /* Pekare till ljuddata */
    unsigned long length;       /* Total längd i bytes */
    unsigned long position;     /* Nuvarande position */
    int loop;                   /* Loopa? */
    int active;                 /* Aktiv kanal? */
    int volume;                 /* Volym 0-255 */
} MixerChannel;

static MixerChannel channels[MIXER_NUM_CHANNELS];

/* Musik-kanal (separat för streaming) */
static FILE *music_file = NULL;
static unsigned long music_data_start = 0;
static unsigned long music_data_size = 0;
static unsigned long music_position = 0;
static int music_loop = 0;
static int music_active = 0;

/* Pre-load buffert för musik (undviker långsam fread per sample) */
#define MUSIC_PRELOAD_SIZE 8192
static unsigned char music_preload[MUSIC_PRELOAD_SIZE];
static unsigned int music_preload_pos = 0;
static unsigned int music_preload_len = 0;

/* IRQ-hantering */
static void (interrupt far *old_irq)(void) = NULL;
static volatile int buffer_ready = 0;
static int irq_num = 7;
static int mixer_running = 0;

/* Debug */
static FILE *dbg = NULL;

/* ========== DEBUG ========== */

static void mixer_debug(const char *msg)
{
    if (dbg == NULL) {
        dbg = fopen("MIXER.TXT", "w");
    }
    if (dbg) {
        fprintf(dbg, "%s\n", msg);
        fflush(dbg);
    }
}

/* ========== DSP FUNKTIONER ========== */

static void sb_write(unsigned char val)
{
    while (inp(SB_WRITE) & 0x80);
    outp(SB_WRITE, val);
}

static int sb_reset(void)
{
    int i;
    
    outp(SB_RESET, 1);
    for (i = 0; i < 100; i++) inp(SB_RESET);
    outp(SB_RESET, 0);
    
    for (i = 0; i < 1000; i++) {
        if (inp(SB_STATUS) & 0x80) {
            if (inp(SB_READ) == 0xAA) {
                return 1;
            }
        }
    }
    return 0;
}

/* ========== DMA FUNKTIONER ========== */

static void dma_setup_autoinit(unsigned long addr, unsigned int total_len)
{
    unsigned char page;
    unsigned int offset;
    
    page = (unsigned char)((addr >> 16) & 0x0F);
    offset = (unsigned int)(addr & 0xFFFF);
    total_len--;  /* DMA vill ha length-1 */
    
    outp(DMA_MASK, 0x05);       /* Maskera kanal 1 */
    outp(DMA_FLIP, 0);          /* Reset flip-flop */
    outp(DMA_MODE, 0x59);       /* Auto-init, single, read, kanal 1 */
    outp(DMA_ADDR, offset & 0xFF);
    outp(DMA_ADDR, (offset >> 8) & 0xFF);
    outp(DMA_PAGE, page);
    outp(DMA_FLIP, 0);
    outp(DMA_COUNT, total_len & 0xFF);
    outp(DMA_COUNT, (total_len >> 8) & 0xFF);
    outp(DMA_MASK, 0x01);       /* Avmaskera kanal 1 */
}

/* ========== IRQ HANDLER ========== */

static void interrupt far mixer_irq(void)
{
    /* Acknowledge DSP */
    inp(SB_STATUS);
    
    /* Signalera att buffert är klar */
    buffer_ready = 1;
    
    /* Skicka EOI till PIC */
    outp(PIC_CMD, PIC_EOI);
}

/* ========== MIXER FUNKTIONER ========== */

/*
 * mixer_fill_buffer - Fyller en buffert med mixad audio
 * 
 * Läser musik från fil och mixar med aktiva SFX-kanaler.
 */
/* Ladda musik-data till pre-load buffert */
static void music_refill_preload(void)
{
    if (music_file == NULL) return;
    
    music_preload_len = fread(music_preload, 1, MUSIC_PRELOAD_SIZE, music_file);
    music_preload_pos = 0;
}

static void mixer_fill_buffer(unsigned char far *buf)
{
    unsigned int i, ch;
    int sample;
    int music_sample;
    int sfx_sample;
    
    for (i = 0; i < MIXER_BUFFER_SIZE; i++) {
        /* Börja med tystnad (128 = mittenvärde för 8-bit unsigned) */
        sample = 0;
        
        /* Läs musik-sample från pre-load buffert */
        if (music_active) {
            if (music_preload_pos < music_preload_len) {
                music_sample = (int)music_preload[music_preload_pos] - 128;
                sample += music_sample;
                music_preload_pos++;
                music_position++;
            } else if (music_file != NULL) {
                /* Behöver ladda mer data */
                if (music_position >= music_data_size) {
                    if (music_loop) {
                        fseek(music_file, music_data_start, SEEK_SET);
                        music_position = 0;
                    } else {
                        music_active = 0;
                    }
                }
                if (music_active) {
                    music_refill_preload();
                    if (music_preload_len > 0) {
                        music_sample = (int)music_preload[music_preload_pos] - 128;
                        sample += music_sample;
                        music_preload_pos++;
                        music_position++;
                    }
                }
            }
        }
        
        /* Mixa in SFX-kanaler */
        for (ch = 0; ch < MIXER_NUM_CHANNELS; ch++) {
            if (channels[ch].active && channels[ch].data != NULL) {
                sfx_sample = (int)channels[ch].data[channels[ch].position] - 128;
                sfx_sample = (sfx_sample * channels[ch].volume) / 256;
                sample += sfx_sample;
                
                channels[ch].position++;
                if (channels[ch].position >= channels[ch].length) {
                    if (channels[ch].loop) {
                        channels[ch].position = 0;
                    } else {
                        channels[ch].active = 0;
                    }
                }
            }
        }
        
        /* Clipping och konvertera tillbaka till unsigned */
        if (sample > 127) sample = 127;
        if (sample < -128) sample = -128;
        buf[i] = (unsigned char)(sample + 128);
    }
}

/* ========== PUBLIKA FUNKTIONER ========== */

int mixer_init(void)
{
    unsigned long addr_a, addr_b;
    unsigned int seg, off;
    unsigned char mask;
    int i;
    
    mixer_debug("=== mixer_init ===");
    
    /* Reset DSP */
    if (!sb_reset()) {
        mixer_debug("ERROR: Sound Blaster ej hittad");
        return 0;
    }
    mixer_debug("Sound Blaster OK");
    
    /* Allokera double-buffer (8KB totalt, 4KB per buffert) */
    /* Måste vara i samma 64KB-page */
    buffer_a = (unsigned char far *)_fmalloc(MIXER_BUFFER_SIZE * 2 + 256);
    if (buffer_a == NULL) {
        mixer_debug("ERROR: Kunde inte allokera buffer");
        return 0;
    }
    
    /* Justera så att buffertarna inte korsar 64KB-gräns */
    seg = FP_SEG(buffer_a);
    off = FP_OFF(buffer_a);
    addr_a = ((unsigned long)seg << 4) + off;
    
    /* Om buffer_a korsar gräns, justera */
    if ((addr_a & 0xFFFF) + (MIXER_BUFFER_SIZE * 2) > 0x10000) {
        /* Flytta till nästa page-gräns */
        addr_a = (addr_a + 0x10000) & 0xFFFF0000UL;
        buffer_a = (unsigned char far *)MK_FP((unsigned int)(addr_a >> 4), 0);
    }
    
    buffer_b = buffer_a + MIXER_BUFFER_SIZE;
    
    mixer_debug("Buffertar allokerade");
    
    /* Initiera kanaler */
    for (i = 0; i < MIXER_NUM_CHANNELS; i++) {
        channels[i].data = NULL;
        channels[i].length = 0;
        channels[i].position = 0;
        channels[i].loop = 0;
        channels[i].active = 0;
        channels[i].volume = 255;
    }
    
    /* Installera IRQ-handler */
    irq_num = 7;  /* Standard SB IRQ */
    old_irq = _dos_getvect(0x08 + irq_num);
    _dos_setvect(0x08 + irq_num, mixer_irq);
    
    /* Aktivera IRQ i PIC */
    mask = inp(PIC_MASK);
    mask &= ~(1 << irq_num);
    outp(PIC_MASK, mask);
    
    mixer_debug("IRQ installerad");
    
    /* Slå på speaker */
    sb_write(0xD1);
    
    mixer_running = 0;
    
    return 1;
}

void mixer_close(void)
{
    unsigned char mask;
    
    mixer_debug("=== mixer_close ===");
    
    /* Stoppa uppspelning */
    mixer_stop();
    
    /* Stäng av speaker */
    sb_write(0xD3);
    
    /* Återställ IRQ */
    if (old_irq != NULL) {
        mask = inp(PIC_MASK);
        mask |= (1 << irq_num);
        outp(PIC_MASK, mask);
        _dos_setvect(0x08 + irq_num, old_irq);
    }
    
    /* Frigör minne */
    if (buffer_a != NULL) {
        _ffree(buffer_a);
        buffer_a = NULL;
        buffer_b = NULL;
    }
    
    /* Stäng musik-fil */
    if (music_file != NULL) {
        fclose(music_file);
        music_file = NULL;
    }
    
    /* Stäng debug */
    if (dbg != NULL) {
        fclose(dbg);
        dbg = NULL;
    }
}

void mixer_start(void)
{
    unsigned long addr;
    unsigned int seg, off;
    unsigned char tc;
    
    if (mixer_running) return;
    
    mixer_debug("mixer_start");
    
    /* Fyll båda buffertarna */
    mixer_fill_buffer(buffer_a);
    mixer_fill_buffer(buffer_b);
    
    current_play = buffer_a;
    current_fill = buffer_b;
    
    /* Beräkna fysisk adress för buffer_a */
    seg = FP_SEG(buffer_a);
    off = FP_OFF(buffer_a);
    addr = ((unsigned long)seg << 4) + off;
    
    /* Konfigurera DMA för auto-init (båda buffertarna) */
    dma_setup_autoinit(addr, MIXER_BUFFER_SIZE * 2);
    
    /* Sätt sample rate */
    tc = (unsigned char)(256 - (1000000UL / MIXER_SAMPLE_RATE));
    sb_write(0x40);
    sb_write(tc);
    
    /* Starta auto-init DMA playback */
    /* 0x1C = 8-bit auto-init DMA */
    sb_write(0x48);  /* Set DMA block size */
    sb_write((MIXER_BUFFER_SIZE - 1) & 0xFF);
    sb_write(((MIXER_BUFFER_SIZE - 1) >> 8) & 0xFF);
    sb_write(0x1C);  /* 8-bit auto-init DMA output */
    
    mixer_running = 1;
    buffer_ready = 0;
}

void mixer_stop(void)
{
    if (!mixer_running) return;
    
    mixer_debug("mixer_stop");
    
    /* Stoppa DMA */
    sb_write(0xD0);  /* Halt DMA */
    sb_write(0xDA);  /* Exit auto-init */
    
    mixer_running = 0;
}

void mixer_update(void)
{
    unsigned char far *temp;
    
    if (!mixer_running) return;
    
    /* Kolla om buffert är klar */
    if (buffer_ready) {
        buffer_ready = 0;
        
        /* Byt buffertar */
        temp = current_play;
        current_play = current_fill;
        current_fill = temp;
        
        /* Fyll den nya fill-bufferten */
        mixer_fill_buffer(current_fill);
    }
}

/* ========== MUSIK FUNKTIONER ========== */

int mixer_play_music(const char *filename, int loop)
{
    char chunk_id[4];
    unsigned long chunk_size;
    int found_data = 0;
    
    mixer_debug("mixer_play_music");
    
    /* Stäng eventuell tidigare musik */
    if (music_file != NULL) {
        fclose(music_file);
        music_file = NULL;
    }
    
    music_file = fopen(filename, "rb");
    if (music_file == NULL) {
        mixer_debug("ERROR: Kunde inte öppna fil");
        return 0;
    }
    
    /* Parsa WAV-header */
    fread(chunk_id, 1, 4, music_file);
    if (memcmp(chunk_id, "RIFF", 4) != 0) {
        fclose(music_file);
        music_file = NULL;
        return 0;
    }
    
    fseek(music_file, 8, SEEK_SET);  /* Hoppa till "WAVE" */
    fread(chunk_id, 1, 4, music_file);
    if (memcmp(chunk_id, "WAVE", 4) != 0) {
        fclose(music_file);
        music_file = NULL;
        return 0;
    }
    
    /* Hitta data-chunk */
    while (fread(chunk_id, 1, 4, music_file) == 4) {
        fread(&chunk_size, 4, 1, music_file);
        
        if (memcmp(chunk_id, "data", 4) == 0) {
            music_data_start = ftell(music_file);
            music_data_size = chunk_size;
            found_data = 1;
            break;
        }
        
        fseek(music_file, chunk_size, SEEK_CUR);
    }
    
    if (!found_data) {
        fclose(music_file);
        music_file = NULL;
        return 0;
    }
    
    music_position = 0;
    music_loop = loop;
    
    /* Pre-load första bufferten */
    music_refill_preload();
    
    music_active = 1;
    
    mixer_debug("Musik startad");
    
    return 1;
}

void mixer_stop_music(void)
{
    music_active = 0;
}

/* ========== SFX FUNKTIONER ========== */

int mixer_play_sfx(unsigned char far *data, unsigned long length, int loop)
{
    int i;
    unsigned int j;
    int sample, sfx_sample;
    
    /* Hitta ledig kanal */
    for (i = 0; i < MIXER_NUM_CHANNELS; i++) {
        if (!channels[i].active) {
            channels[i].data = data;
            channels[i].length = length;
            channels[i].position = 0;
            channels[i].loop = loop;
            channels[i].volume = 128;  /* 75% volym */
            channels[i].active = 1;
            
            /* DIREKT-TRIGGER: Mixa in SFX omedelbart i fill-buffert */
            if (current_fill != NULL && mixer_running) {
                for (j = 0; j < MIXER_BUFFER_SIZE && j < length; j++) {
                    sample = (int)current_fill[j] - 128;
                    sfx_sample = ((int)data[j] - 128) * channels[i].volume / 256;
                    sample += sfx_sample;
                    if (sample > 127) sample = 127;
                    if (sample < -128) sample = -128;
                    current_fill[j] = (unsigned char)(sample + 128);
                }
                /* Hoppa förbi det som redan mixats för att undvika dubbel uppspelning */
                channels[i].position = j;
            }
            
            return i;
        }
    }
    
    return -1;  /* Ingen ledig kanal */
}

void mixer_stop_sfx(int channel)
{
    if (channel >= 0 && channel < MIXER_NUM_CHANNELS) {
        channels[channel].active = 0;
    }
}
