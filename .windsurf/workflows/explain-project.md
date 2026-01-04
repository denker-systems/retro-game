---
name: explain-project
description: Förklarar projektets arkitektur, moduler och hur allt hänger ihop
---

# Förklara Projektet

## Steg 1: Ge översikt
Förklara projektets syfte och mål:
- Detta är ett DOS-plattformsspel skrivet i C
- Målplattform: MS-DOS med VGA-grafik
- Kompilator: Open Watcom

## Steg 2: Förklara arkitekturen
Gå igenom modulstrukturen:

`
src/
 main.c      - Spelloop och initiering
 vga.c/h     - VGA Mode 13h grafik
 input.c/h   - Tangentbordshantering
 player.c/h  - Spelarlogik och fysik
 level.c/h   - Nivådata och kollisioner
 types.h     - Gemensamma typer
`

## Steg 3: Förklara dataflödet
Visa hur data flödar genom systemet:
1. input.c läser tangentbord -> InputState
2. player.c tar InputState -> uppdaterar Player
3. level.c kollar kollisioner
4. vga.c ritar allt till skärmen

## Steg 4: Förklara spelloopen
Beskriv INPUT -> UPDATE -> RENDER cykeln i main.c

## Steg 5: Fråga om fördjupning
Fråga användaren om de vill veta mer om:
- En specifik modul?
- Hur VGA-programmering fungerar?
- Hur kollisionsdetektion fungerar?
- Hur fysiken implementeras?
