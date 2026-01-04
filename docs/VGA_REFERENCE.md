# VGA Mode 13h Referens

## Snabbreferens

| Egenskap | Värde |
|----------|-------|
| Upplösning | 320 × 200 pixlar |
| Färger | 256 (8-bit palette) |
| Framebuffer | 0xA0000 |
| Storlek | 64000 bytes |
| BIOS-läge | 0x13 |

## Aktivera Mode 13h

```c
#include <dos.h>

void set_mode_13h(void) {
    union REGS regs;
    regs.h.ah = 0x00;  /* Funktion: Set Video Mode */
    regs.h.al = 0x13;  /* Mode 13h */
    int86(0x10, &regs, &regs);
}
```

## Återställa textläge

```c
void set_text_mode(void) {
    union REGS regs;
    regs.h.ah = 0x00;
    regs.h.al = 0x03;  /* Mode 3 = 80x25 text */
    int86(0x10, &regs, &regs);
}
```

## Rita en pixel

```c
byte far *vga = (byte far *)0xA0000000L;

void put_pixel(int x, int y, byte color) {
    vga[y * 320 + x] = color;
}
```

## Standardpaletten (första 16 färger)

| Index | Färg | RGB (ungefär) |
|-------|------|---------------|
| 0 | Svart | 0, 0, 0 |
| 1 | Mörkblå | 0, 0, 170 |
| 2 | Mörkgrön | 0, 170, 0 |
| 3 | Mörkcyan | 0, 170, 170 |
| 4 | Mörkröd | 170, 0, 0 |
| 5 | Mörkmagenta | 170, 0, 170 |
| 6 | Brun | 170, 85, 0 |
| 7 | Ljusgrå | 170, 170, 170 |
| 8 | Mörkgrå | 85, 85, 85 |
| 9 | Ljusblå | 85, 85, 255 |
| 10 | Ljusgrön | 85, 255, 85 |
| 11 | Ljuscyan | 85, 255, 255 |
| 12 | Ljusröd | 255, 85, 85 |
| 13 | Ljusmagenta | 255, 85, 255 |
| 14 | Gul | 255, 255, 85 |
| 15 | Vit | 255, 255, 255 |

## VSync

```c
#include <conio.h>

#define VGA_STATUS_REG 0x3DA
#define VSYNC_BIT 0x08

void wait_vsync(void) {
    /* Vänta tills eventuell pågående retrace är klar */
    while (inp(VGA_STATUS_REG) & VSYNC_BIT);
    
    /* Vänta tills retrace börjar */
    while (!(inp(VGA_STATUS_REG) & VSYNC_BIT));
}
```

## Ändra palette

```c
#include <conio.h>

void set_palette_color(byte index, byte r, byte g, byte b) {
    outp(0x3C8, index);    /* Palette index */
    outp(0x3C9, r >> 2);   /* Röd (0-63) */
    outp(0x3C9, g >> 2);   /* Grön (0-63) */
    outp(0x3C9, b >> 2);   /* Blå (0-63) */
}
```

**OBS:** VGA-paletten använder 6-bit värden (0-63), inte 8-bit.
Därför dividerar vi med 4 (>> 2).

## Dubbelbuffring

```c
#include <malloc.h>
#include <string.h>

byte far *backbuffer;
byte far *vga = (byte far *)0xA0000000L;

void init_double_buffer(void) {
    backbuffer = _fmalloc(64000);
}

void flip(void) {
    _fmemcpy(vga, backbuffer, 64000);
}

void clear_buffer(byte color) {
    _fmemset(backbuffer, color, 64000);
}
```

## VGA-register

| Port | Läs/Skriv | Funktion |
|------|-----------|----------|
| 0x3C8 | W | Palette index (för skrivning) |
| 0x3C9 | R/W | Palette RGB data |
| 0x3DA | R | Input Status Register 1 |

### Status Register 1 (0x3DA)

| Bit | Betydelse |
|-----|-----------|
| 0 | Display Enable (0 = aktiv visning) |
| 3 | Vertical Retrace (1 = retrace pågår) |

## Minnesberäkning

```
Pixel offset = Y * 320 + X

Exempel: pixel (100, 50)
Offset = 50 * 320 + 100 = 16100
Adress = 0xA0000 + 16100 = 0xA3EF4
```

## Begränsningar

- Endast 64KB framebuffer (320×200 = 64000 bytes)
- Ingen hårdvarustöd för scrollning
- Ingen hårdvarustöd för sprites
- Palette-baserat (svårt med fotorealistisk grafik)
- Kräver manuell dubbelbuffring
