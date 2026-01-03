/*
 * vga.h - VGA Mode 13h grafikfunktioner
 */
#ifndef VGA_H
#define VGA_H

#include "types.h"

void vga_init(void);
void vga_close(void);
void vga_clear(byte color);
void vga_draw_rect(int x, int y, int w, int h, byte color);
void vga_flip(void);
void vga_vsync(void);

#endif
