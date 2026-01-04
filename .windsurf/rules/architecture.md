---
trigger: always_on
---

# Projektarkitektur

<overview>
Projektet använder en klassisk C-arkitektur med separata mappar
för källkod och header-filer. Detta gör det tydligt vilka filer
som är implementation och vilka som är gränssnitt.
</overview>

<folder_structure>
retro-game/
 src/                    # Källkod (implementation)
    main.c              # Entry point och spelloop
    vga.c               # VGA-grafikimplementation
    input.c             # Tangentbordshantering
    player.c            # Spelarlogik
    level.c             # Nivådata och kollision

 include/                # Header-filer (gränssnitt)
    types.h             # Gemensamma typer och konstanter
    vga.h               # VGA-funktionsdeklarationer
    input.h             # Input-strukturer och funktioner
    player.h            # Player-struktur och funktioner
    level.h             # Nivåfunktioner

 assets/                 # Grafik, ljud, nivådata (framtida)
    sprites/
    sounds/
    levels/

 docs/                   # Dokumentation
    README.md

 .windsurf/              # Windsurf-konfiguration
     rules/
     workflows/
</folder_structure>

<module_responsibilities>
Varje modul har ETT tydligt ansvar:

| Modul | Ansvar |
|-------|--------|
| main.c | Spelloop, initiering, koordinering |
| vga.c | ALL VGA-kommunikation |
| input.c | ALL tangentbordsläsning |
| player.c | Spelarens fysik och rendering |
| level.c | Världsdata och kollision |

Moduler ska INTE:
- Känna till varandras interna implementation
- Direkt modifiera varandras data
- Ha cirkulära beroenden
</module_responsibilities>

<header_vs_source>
## Header-filer (.h) - Gränssnitt
Innehåller:
- Strukturdefinitioner (typedef struct)
- Funktionsdeklarationer
- Publika konstanter (#define)
- Dokumentation av API:et

INTE:
- Implementation (funktionskroppar)
- Interna variabler
- Privata funktioner

## Källfiler (.c) - Implementation
Innehåller:
- Inkludering av egen header
- static-variabler (privata)
- static-funktioner (interna hjälpfunktioner)
- Funktionsimplementationer
</header_vs_source>

<dependency_rules>
## Beroenderegler

1. main.c kan inkludera ALLA headers
2. player.c inkluderar: player.h, level.h, vga.h, types.h
3. level.c inkluderar: level.h, vga.h
4. vga.c inkluderar: vga.h, types.h
5. input.c inkluderar: input.h

## Förbjudet
- vga.c får INTE inkludera player.h
- level.c får INTE inkludera input.h
- Cirkulära beroenden är FÖRBJUDNA
</dependency_rules>

<include_path>
Kompilera med include-sökväg:
wcl -0 -ms -i=include src\*.c -fe=game.exe

Inkludering i källfiler:
#include "types.h"    /* Hittas via -i=include */
</include_path>

<game_loop>
Standard spelloop i main.c:

while (!quit) {
    /* 1. INPUT */
    input_update(&input);
    
    /* 2. UPDATE */
    player_update(&player, input);
    level_update();
    
    /* 3. RENDER */
    vga_clear(SKY_COLOR);
    level_draw();
    player_draw(&player);
    
    /* 4. PRESENT */
    vga_vsync();
    vga_flip();
}
</game_loop>
