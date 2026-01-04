/*
 * sbtest.c - Minimal Sound Blaster test
 * 
 * Testar grundläggande SB-uppspelning med "nagging DSP" teknik.
 * Kompilera separat: wcl -0 -ms sbtest.c -fe=sbtest.exe
 */

#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <malloc.h>

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

/* Skriv till DSP */
void sb_write(unsigned char val)
{
    while (inp(SB_WRITE) & 0x80);  /* Vänta tills redo */
    outp(SB_WRITE, val);
}

/* Reset DSP */
int sb_reset(void)
{
    int i;
    
    outp(SB_RESET, 1);
    for (i = 0; i < 100; i++) inp(SB_RESET);  /* Delay */
    outp(SB_RESET, 0);
    
    for (i = 0; i < 1000; i++) {
        if (inp(SB_STATUS) & 0x80) {
            if (inp(SB_READ) == 0xAA) {
                return 1;  /* OK */
            }
        }
    }
    return 0;  /* Misslyckades */
}

/* Konfigurera DMA kanal 1 */
void dma_setup(unsigned long addr, unsigned int len)
{
    unsigned char page;
    unsigned int offset;
    
    page = (unsigned char)((addr >> 16) & 0x0F);
    offset = (unsigned int)(addr & 0xFFFF);
    len--;  /* DMA vill ha length-1 */
    
    outp(DMA_MASK, 0x05);     /* Maskera kanal 1 */
    outp(DMA_FLIP, 0);        /* Reset flip-flop */
    outp(DMA_MODE, 0x49);     /* Single, read, kanal 1 */
    outp(DMA_ADDR, offset & 0xFF);
    outp(DMA_ADDR, (offset >> 8) & 0xFF);
    outp(DMA_PAGE, page);
    outp(DMA_FLIP, 0);
    outp(DMA_COUNT, len & 0xFF);
    outp(DMA_COUNT, (len >> 8) & 0xFF);
    outp(DMA_MASK, 0x01);     /* Avmaskera kanal 1 */
}

int main(void)
{
    unsigned char far *buffer;
    unsigned long phys_addr;
    unsigned int seg, off;
    unsigned int i;
    unsigned char tc;
    
    printf("Sound Blaster Test\n");
    printf("==================\n\n");
    
    /* Reset DSP */
    if (!sb_reset()) {
        printf("ERROR: Kunde inte hitta Sound Blaster!\n");
        return 1;
    }
    printf("Sound Blaster hittad pa port 0x220\n");
    
    /* Allokera buffert */
    buffer = (unsigned char far *)_fmalloc(8000);
    if (buffer == NULL) {
        printf("ERROR: Kunde inte allokera minne!\n");
        return 1;
    }
    
    /* Fyll med sinusvåg (8-bit unsigned, 128 = tyst) */
    printf("Genererar sinusvag...\n");
    for (i = 0; i < 8000; i++) {
        /* Enkel sägtandsvåg som test */
        buffer[i] = (unsigned char)(128 + (i % 256) - 128);
    }
    
    /* Beräkna fysisk adress */
    seg = FP_SEG(buffer);
    off = FP_OFF(buffer);
    phys_addr = ((unsigned long)seg << 4) + off;
    
    printf("Buffer: %04X:%04X = 0x%lX\n", seg, off, phys_addr);
    
    /* Kontrollera 64KB-gräns */
    if ((phys_addr & 0xFFFF) + 8000 > 0x10000) {
        printf("ERROR: Buffer korsar 64KB-grans!\n");
        _ffree(buffer);
        return 1;
    }
    
    /* Slå på speaker */
    sb_write(0xD1);
    printf("Speaker ON\n");
    
    /* Konfigurera DMA */
    dma_setup(phys_addr, 8000);
    printf("DMA konfigurerad\n");
    
    /* Sätt time constant för 11025 Hz */
    /* tc = 256 - (1000000 / rate) = 256 - 90 = 166 */
    tc = 166;
    sb_write(0x40);
    sb_write(tc);
    printf("Time constant: %d (11025 Hz)\n", tc);
    
    /* Starta uppspelning med 0x14 */
    sb_write(0x14);
    sb_write((8000 - 1) & 0xFF);
    sb_write(((8000 - 1) >> 8) & 0xFF);
    printf("Spelar... (tryck tangent for att avsluta)\n");
    
    /* Vänta på tangenttryckning */
    while (!kbhit()) {
        /* Här kunde vi "nagga" DSP med nya 0x14-kommandon */
    }
    getch();
    
    /* Stoppa */
    sb_write(0xD0);  /* Halt DMA */
    sb_write(0xD3);  /* Speaker off */
    
    _ffree(buffer);
    printf("\nKlar!\n");
    
    return 0;
}
