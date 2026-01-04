# Lärresurser - DOS Spelprogrammering

## Koncept att förstå

### 1. VGA Mode 13h

**Vad är det?**
Ett grafikläge som ger 320×200 pixlar med 256 färger.
Det var standardläget för DOS-spel 1990-1995.

**Varför Mode 13h?**
- Enkelt: Linjär framebuffer (pixel[y*320+x] = färg)
- Snabbt: Direkt minnesåtkomst
- Tillräckligt: 256 färger räckte för 2D-spel

**Framebuffer:**
```
Adress 0xA0000 i minnet = pixel (0,0)
Adress 0xA0001 = pixel (1,0)
...
Adress 0xA0140 (320 dec) = pixel (0,1)
```

### 2. Dubbelbuffring

**Problem utan dubbelbuffring:**
Om vi ritar direkt till skärmen medan den uppdateras
ser användaren "tearing" - halva bilden är gammal, halva ny.

**Lösning:**
1. Rita till en buffert i RAM (inte skärmen)
2. Vänta på vertical retrace (skärmen ritar om sig)
3. Kopiera hela bufferten till VGA-minnet på en gång

```
RAM-buffert ──(vsync)──▶ VGA-minne ──▶ Skärm
```

### 3. Vertical Sync (VSync)

**Vad händer i en CRT-skärm?**
En elektronstråle ritar bilden rad för rad, uppifrån och ner.
När den når botten flyger den tillbaka till toppen (retrace).

**Varför vänta på vsync?**
Under retrace ritas ingenting på skärmen.
Det är det perfekta tillfället att uppdatera VGA-minnet!

**Hur?**
```c
while (inp(0x3DA) & 8);   /* Vänta tills retrace slutar */
while (!(inp(0x3DA) & 8)); /* Vänta tills retrace börjar */
```

### 4. Far Pointers

**Real Mode minnesmodell:**
8086-processorn adresserar minne med segment:offset.
Max 64KB per segment, men vi kan nå 1MB totalt.

**Near pointer:** Bara offset (inom samma segment)
**Far pointer:** Segment + offset (kan nå var som helst)

```c
byte far *vga = (byte far *)0xA0000000L;
/*    ^^^
      Far pointer behövs för att nå VGA-minnet
      som ligger utanför vårt datasegment */
```

### 5. AABB Kollision

**AABB = Axis-Aligned Bounding Box**
Rektanglar som inte är roterade.

**Kollisionstest:**
Två rektanglar kolliderar OM de överlappar på BÅDE X och Y.

```
Kollision:            Ingen kollision:

┌────┐                ┌────┐
│ A ┌┼───┐            │ A  │     ┌────┐
└───┼┘ B │            └────┘     │ B  │
    └────┘                       └────┘
```

**Kod:**
```c
if (a.left < b.right && a.right > b.left &&
    a.top < b.bottom && a.bottom > b.top) {
    /* Kollision! */
}
```

### 6. Spelloop

**Alla spel har samma grundstruktur:**

```c
while (running) {
    process_input();   /* Läs tangentbord/mus */
    update_game();     /* Fysik, AI, kollision */
    render_frame();    /* Rita allt */
}
```

**Varför denna ordning?**
- Input först: så update har färsk data
- Update före render: så vi ritar rätt tillstånd
- Render sist: så spelaren ser resultatet

## Övningar

### Nivå 1: Nybörjare

1. **Ändra färger**
   - Öppna `src/vga.c`, hitta `vga_clear(1)`
   - Byt 1 till annan färg (0-255)
   - Kompilera och se skillnaden

2. **Ändra fysik**
   - Öppna `src/player.c`
   - Ändra `GRAVITY`, `JUMP_FORCE`, `MOVE_SPEED`
   - Hur känns spelet med GRAVITY=2? JUMP_FORCE=-12?

3. **Flytta plattformar**
   - Öppna `src/level.c`
   - Ändra koordinater i `platforms[]`
   - Skapa din egen nivålayout

### Nivå 2: Mellanliggande

4. **Lägg till ny plattform**
   - Utöka `platforms[]` arrayen
   - Öka `NUM_PLATFORMS`
   - Testa att den nya plattformen fungerar

5. **Animerad spelare**
   - Rita olika sprites baserat på `player->vy`
   - Positiv vy = faller (nedåtpekande sprite)
   - Negativ vy = hoppar (uppåtpekande sprite)

6. **Samlarobjekt**
   - Skapa en array med "mynt" (x, y, collected)
   - Rita mynt som inte är plockade
   - Kolla kollision med spelaren
   - Räkna poäng

### Nivå 3: Avancerad

7. **Rörlig plattform**
   - Lägg till `vx` till en plattform
   - Uppdatera plattformens X varje frame
   - Byt riktning vid kanterna

8. **Enkel fiende**
   - Skapa `enemy.c/h`
   - Fiende som går fram och tillbaka
   - Game over vid kollision med spelaren

9. **Kamera/Scrollning**
   - Nivå större än skärmen
   - Kamera följer spelaren
   - Rita objekt relativt till kameran

## Resurser

### Böcker
- "Game Programming Patterns" - Robert Nystrom
- "Tricks of the Game Programming Gurus" (1994)

### Webbsidor
- https://www.brackeen.com/vga/ - VGA programmering
- https://wiki.osdev.org/VGA_Hardware - Teknisk referens
- https://moddingwiki.shikadi.net/ - DOS spelmodding

### Källkod att studera
- Commander Keen (id Software)
- Duke Nukem 1/2 (Apogee)
- Jazz Jackrabbit (Epic MegaGames)

## DOS-historik

| År | Händelse |
|----|----------|
| 1981 | IBM PC lanseras med PC-DOS |
| 1984 | EGA-grafik (16 färger) |
| 1987 | VGA-grafik (256 färger) |
| 1990 | Commander Keen revolutionerar sidoscrollning |
| 1992 | Wolfenstein 3D - första FPS |
| 1993 | DOOM - definierar genren |
| 1995 | Windows 95 börjar ersätta DOS |
