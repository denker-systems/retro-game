/*
 * vga.c - VGA Mode 13h implementation
 * 
 * Mode 13h ger oss:
 * - 320x200 pixels
 * - 256 färger (8-bit)
 * - Linjär framebuffer på adress 0xA0000
 */
#include <dos.h>
#include <string.h>
#include <malloc.h>
#include "vga.h"

static byte far *VGA = (byte far *)0xA0000000L;
static byte far *buffer = NULL;

void vga_init(void) {
    union REGS regs;
    
    /* Allokera dubbelbuffer (64000 bytes = 320*200) */
    buffer = (byte far *)_fmalloc(64000U);
    
    /* Sätt Mode 13h via BIOS interrupt 10h */
    regs.h.ah = 0x00;
    regs.h.al = 0x13;
    int86(0x10, &regs, &regs);
}

void vga_close(void) {
    union REGS regs;
    
    /* Tillbaka till textläge (Mode 03h) */
    regs.h.ah = 0x00;
    regs.h.al = 0x03;
    int86(0x10, &regs, &regs);
    
    if (buffer) {
        _ffree(buffer);
        buffer = NULL;
    }
}

void vga_clear(byte color) {
    _fmemset(buffer, color, 64000U);
}

void vga_draw_rect(int x, int y, int w, int h, byte color) {
    int i, j;
    byte far *dest;
    
    for (j = 0; j < h; j++) {
        if (y + j < 0 || y + j >= SCREEN_H) continue;
        dest = buffer + (y + j) * SCREEN_W + x;
        for (i = 0; i < w; i++) {
            if (x + i >= 0 && x + i < SCREEN_W) {
                dest[i] = color;
            }
        }
    }
}

void vga_flip(void) {
    /* Kopiera buffer till VGA-minnet */
    _fmemcpy(VGA, buffer, 64000U);
}

void vga_vsync(void) {
    /* Vänta på vertical retrace för att undvika flimmer */
    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}
