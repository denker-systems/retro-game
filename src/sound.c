/*
 * sound.c - Ljudsystem (Sound Blaster + PC Speaker fallback)
 * 
 * SYFTE:
 *   Spelar ljudeffekter via Sound Blaster om tillgänglig,
 *   annars PC Speaker som fallback.
 *   
 * HÅRDVARA:
 *   Sound Blaster: DMA-baserad 8-bit PCM uppspelning
 *   PC Speaker: PIT Timer 2 fyrkantvåg
 */

#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "sound.h"
#include "mixer.h"

/* Mixer status */
static int mixer_initialized = 0;

/* Debug-loggning */
static FILE *dbg = NULL;

static void debug_log(const char *msg)
{
    if (dbg == NULL) {
        dbg = fopen("DEBUG.TXT", "w");
    }
    if (dbg) {
        fprintf(dbg, "%s\n", msg);
        fflush(dbg);
    }
}

static void debug_log_hex(const char *msg, unsigned long val)
{
    if (dbg == NULL) {
        dbg = fopen("DEBUG.TXT", "w");
    }
    if (dbg) {
        fprintf(dbg, "%s 0x%lX\n", msg, val);
        fflush(dbg);
    }
}

static void debug_log_int(const char *msg, int val)
{
    if (dbg == NULL) {
        dbg = fopen("DEBUG.TXT", "w");
    }
    if (dbg) {
        fprintf(dbg, "%s %d\n", msg, val);
        fflush(dbg);
    }
}

/* ========== SOUND BLASTER ========== */

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

/* Globala variabler */
int sb_detected = 0;
int sb_base_port = 0x220;
int sb_irq = 7;
int sb_dma = 1;

/* DMA-buffert */
static unsigned char far *dma_buffer = NULL;

/* ========== PC SPEAKER ========== */

#define PIT_FREQUENCY 1193180
#define PIT_DATA    0x42
#define PIT_CONTROL 0x43
#define SPEAKER_PORT 0x61

/* ========== SOUND BLASTER FUNKTIONER ========== */

static void sb_write_dsp(unsigned char val)
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

static void dma_setup(unsigned long addr, unsigned int len)
{
    unsigned char page;
    unsigned int offset;
    
    page = (unsigned char)((addr >> 16) & 0x0F);
    offset = (unsigned int)(addr & 0xFFFF);
    len--;
    
    outp(DMA_MASK, 0x05);
    outp(DMA_FLIP, 0);
    outp(DMA_MODE, 0x49);
    outp(DMA_ADDR, offset & 0xFF);
    outp(DMA_ADDR, (offset >> 8) & 0xFF);
    outp(DMA_PAGE, page);
    outp(DMA_FLIP, 0);
    outp(DMA_COUNT, len & 0xFF);
    outp(DMA_COUNT, (len >> 8) & 0xFF);
    outp(DMA_MASK, 0x01);
}

/* ========== PC SPEAKER FUNKTIONER ========== */

void sound_beep(int freq, int duration_ms)
{
    unsigned int divisor;
    unsigned long i;
    unsigned char old_port;
    
    if (freq == 0) return;
    
    divisor = PIT_FREQUENCY / freq;
    
    outp(PIT_CONTROL, 0xB6);
    outp(PIT_DATA, divisor & 0xFF);
    outp(PIT_DATA, (divisor >> 8) & 0xFF);
    
    old_port = inp(SPEAKER_PORT);
    outp(SPEAKER_PORT, old_port | 0x03);
    
    for (i = 0; i < (unsigned long)duration_ms * 100UL; i++) {
        inp(0x80);
    }
    
    outp(SPEAKER_PORT, old_port);
}

void sound_off(void)
{
    unsigned char port = inp(SPEAKER_PORT);
    outp(SPEAKER_PORT, port & 0xFC);
}

/* ========== PUBLIKA FUNKTIONER ========== */

void sound_init(void)
{
    debug_log("=== sound_init ===");
    sb_detected = 0;
    
    /* Försök hitta Sound Blaster */
    debug_log("Provar SB reset...");
    if (sb_reset()) {
        sb_detected = 1;
        debug_log("SB HITTAD!");
        sb_write_dsp(0xD1);  /* Speaker ON */
        debug_log("Speaker ON");
        
        /* Allokera DMA-buffert */
        dma_buffer = (unsigned char far *)_fmalloc(8192);
        if (dma_buffer) {
            debug_log("DMA-buffert allokerad");
        } else {
            debug_log("ERROR: Kunde inte allokera DMA-buffert!");
        }
    } else {
        debug_log("SB ej hittad - använder PC Speaker");
    }
    debug_log_int("sb_detected =", sb_detected);
}

void sb_play_sample(unsigned char far *data, unsigned int length, unsigned int rate)
{
    unsigned long phys_addr;
    unsigned int seg, off;
    unsigned char tc;
    unsigned int i;
    
    debug_log("--- sb_play_sample ---");
    debug_log_int("length =", length);
    debug_log_int("rate =", rate);
    
    if (!sb_detected || dma_buffer == NULL) {
        debug_log("AVBRYTER: sb_detected=0 eller dma_buffer=NULL");
        return;
    }
    if (length > 8192) length = 8192;
    
    /* Kopiera data till DMA-buffert */
    for (i = 0; i < length; i++) {
        dma_buffer[i] = data[i];
    }
    debug_log("Data kopierad till DMA-buffert");
    
    /* Beräkna fysisk adress */
    seg = FP_SEG(dma_buffer);
    off = FP_OFF(dma_buffer);
    phys_addr = ((unsigned long)seg << 4) + off;
    debug_log_hex("phys_addr =", phys_addr);
    
    /* Kontrollera 64KB-gräns */
    if ((phys_addr & 0xFFFF) + length > 0x10000) {
        debug_log("ERROR: Korsar 64KB-gräns!");
        return;
    }
    
    /* Konfigurera DMA */
    dma_setup(phys_addr, length);
    debug_log("DMA konfigurerad");
    
    /* Time constant: tc = 256 - (1000000 / rate) */
    tc = (unsigned char)(256 - (1000000UL / rate));
    debug_log_int("time_const =", tc);
    sb_write_dsp(0x40);
    sb_write_dsp(tc);
    
    /* Starta uppspelning (0x14 = 8-bit single-cycle) */
    sb_write_dsp(0x14);
    sb_write_dsp((length - 1) & 0xFF);
    sb_write_dsp(((length - 1) >> 8) & 0xFF);
    debug_log("Uppspelning startad!");
}

void sound_close(void)
{
    debug_log("=== sound_close ===");
    
    if (sb_detected) {
        sb_write_dsp(0xD3);  /* Speaker OFF */
        debug_log("SB Speaker OFF");
    }
    
    if (dma_buffer != NULL) {
        _ffree(dma_buffer);
        dma_buffer = NULL;
        debug_log("DMA-buffert frigjord");
    }
    
    if (dbg != NULL) {
        fclose(dbg);
        dbg = NULL;
    }
    
    sound_off();
}

/* Generera enkel ljudeffekt i buffert */
static unsigned char far *sfx_buffer = NULL;

static void generate_sfx(unsigned int freq, unsigned int len)
{
    unsigned int i;
    unsigned int period;
    
    if (sfx_buffer == NULL) {
        sfx_buffer = (unsigned char far *)_fmalloc(4000);
        if (sfx_buffer == NULL) return;
    }
    
    /* Generera fyrkantvåg (lägre amplitud för att passa med musik) */
    period = 11025 / freq;
    for (i = 0; i < len && i < 4000; i++) {
        if ((i / period) % 2) {
            sfx_buffer[i] = 160;  /* Hög (sänkt från 200) */
        } else {
            sfx_buffer[i] = 96;   /* Låg (höjt från 56) */
        }
    }
}

void sound_play(int effect)
{
    debug_log_int("sound_play effect =", effect);
    
    /* Om mixer är igång, använd den för SFX */
    if (mixer_initialized) {
        switch (effect) {
            case SFX_JUMP:
                generate_sfx(800, 2000);
                mixer_play_sfx(sfx_buffer, 2000, 0);
                return;
            case SFX_MENU_MOVE:
                generate_sfx(600, 1500);
                mixer_play_sfx(sfx_buffer, 1500, 0);
                return;
            case SFX_MENU_SELECT:
                generate_sfx(1000, 2000);
                mixer_play_sfx(sfx_buffer, 2000, 0);
                return;
            case SFX_LAND:
                generate_sfx(300, 1500);
                mixer_play_sfx(sfx_buffer, 1500, 0);
                return;
        }
    }
    
    /* Fallback: direkt SB (utan mixer) */
    if (sb_detected) {
        switch (effect) {
            case SFX_JUMP:
                generate_sfx(800, 2000);
                sb_play_sample(sfx_buffer, 2000, 11025);
                return;
            case SFX_MENU_MOVE:
                generate_sfx(600, 1500);
                sb_play_sample(sfx_buffer, 1500, 11025);
                return;
            case SFX_MENU_SELECT:
                generate_sfx(1000, 2000);
                sb_play_sample(sfx_buffer, 2000, 11025);
                return;
            case SFX_LAND:
                generate_sfx(300, 1500);
                sb_play_sample(sfx_buffer, 1500, 11025);
                return;
        }
    }
    
    /* Fallback till PC Speaker */
    switch (effect) {
        case SFX_JUMP:
            sound_beep(800, 30);
            break;
        case SFX_MENU_MOVE:
            sound_beep(600, 20);
            break;
        case SFX_MENU_SELECT:
            sound_beep(1000, 30);
            break;
        case SFX_LAND:
            sound_beep(300, 20);
            break;
    }
}

void sound_jingle(int type)
{
    switch (type) {
        case JINGLE_INTRO:
            sound_beep(523, 80);
            sound_beep(659, 80);
            sound_beep(784, 80);
            sound_beep(1047, 150);
            break;
        case JINGLE_WIN:
            sound_beep(784, 100);
            sound_beep(988, 100);
            sound_beep(1175, 200);
            break;
    }
}

/* ========== MUSIK VIA MIXER ========== */

int music_play(const char *filename, int loop)
{
    debug_log("music_play via mixer");
    
    if (!mixer_initialized) {
        if (mixer_init()) {
            mixer_initialized = 1;
            mixer_start();
            debug_log("Mixer startad");
        } else {
            debug_log("ERROR: Mixer init misslyckades");
            return 0;
        }
    }
    
    return mixer_play_music(filename, loop);
}

void music_stop(void)
{
    if (mixer_initialized) {
        mixer_stop_music();
    }
}

void music_update(void)
{
    if (mixer_initialized) {
        mixer_update();
    }
}

int music_playing(void)
{
    return 0;  /* TODO: implementera */
}
