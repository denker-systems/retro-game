---
trigger: glob
globs: ["*.c", "*.h"]
---

# Pedagogiska Kommentarer

<purpose>
All kod ska ha extremt detaljerade kommentarer för lärande.
Målet är att någon utan förkunskaper ska förstå exakt vad som händer.
</purpose>

<comment_requirements>
- Förklara VARFÖR, inte bara VAD
- Beskriv hårdvaran/systemet bakom varje operation
- Inkludera minnesadresser och register när relevant
- Förklara varje parameter och returvärde
- Beskriv sidoeffekter och beroenden
</comment_requirements>

<file_header>
Varje fil ska börja med:
/*
 * filnamn.c - Kort beskrivning
 * 
 * SYFTE:
 *   Utförlig förklaring av modulens ansvar
 * 
 * HÅRDVARA/SYSTEM:
 *   Vilken hårdvara/DOS-funktion denna modul interagerar med
 * 
 * BEROENDEN:
 *   Vilka andra moduler denna fil använder
 */
</file_header>

<function_comments>
Varje funktion ska ha:
/*
 * funktionsnamn - Vad funktionen gör
 * 
 * PARAMETRAR:
 *   param1 - Beskrivning och giltiga värden
 *   param2 - Beskrivning och giltiga värden
 * 
 * RETURNERAR:
 *   Vad som returneras och möjliga värden
 * 
 * FUNGERAR SÅ HÄR:
 *   Steg-för-steg förklaring av algoritmen
 * 
 * HÅRDVARA:
 *   Vilken hårdvara som påverkas (om relevant)
 * 
 * EXEMPEL:
 *   Exempel på användning
 */
</function_comments>

<inline_comments>
Inuti funktioner:
- Kommentera varje logiskt steg
- Förklara magiska tal och konstanter
- Beskriv minnesoperationer i detalj
- Förklara bitoperationer bit för bit
- Visa beräkningar steg för steg
</inline_comments>

<example>
/*
 * vga_draw_rect - Ritar en fylld rektangel till dubbelbufferten
 * 
 * PARAMETRAR:
 *   x     - X-koordinat för övre vänstra hörnet (0-319)
 *   y     - Y-koordinat för övre vänstra hörnet (0-199)
 *   w     - Bredd i pixlar
 *   h     - Höjd i pixlar
 *   color - Färgindex i VGA-paletten (0-255)
 * 
 * FUNGERAR SÅ HÄR:
 *   1. Loopar genom varje rad (y till y+h)
 *   2. Beräknar minnesoffset: y * 320 + x
 *   3. Fyller w bytes med färgvärdet
 * 
 * VARFÖR 320?
 *   Mode 13h har 320 pixlar per rad.
 *   Varje pixel är 1 byte (256 färger = 8 bit).
 *   Rad 0 börjar på offset 0, rad 1 på 320, osv.
 */
void vga_draw_rect(int x, int y, int w, int h, byte color) {
    int row;           /* Aktuell rad vi ritar */
    byte far *dest;    /* Pekare till destination i bufferten */
    
    /* Loopa genom varje rad i rektangeln */
    for (row = 0; row < h; row++) {
        /* 
         * Beräkna minnesadressen för denna rad:
         * - (y + row) ger aktuell skärmrad
         * - Multiplicera med 320 (bytes per rad)
         * - Addera x för att få startkolumnen
         */
        dest = buffer + (y + row) * SCREEN_W + x;
        
        /*
         * Fyll hela raden med färgen.
         * _fmemset är en far-pointer version av memset
         * som fungerar över 64KB-segmentgränser.
         */
        _fmemset(dest, color, w);
    }
}
</example>

<hardware_explanations>
När du interagerar med hårdvara, förklara:
- Vilken port/adress som används
- Vad varje bit/byte betyder
- Varför operationen görs i just denna ordning
- Vad som händer om man gör fel
</hardware_explanations>
