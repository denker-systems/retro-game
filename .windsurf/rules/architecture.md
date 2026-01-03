---
trigger: always_on
---

# Projektarkitektur

<module_structure>
src/
 main.c      # Spelloop, initiering
 vga.c/h     # VGA Mode 13h grafik
 input.c/h   # Tangentbordshantering
 player.c/h  # Spelarstate och fysik
 level.c/h   # Världsdata och kollision
 types.h     # Delade typer och konstanter
</module_structure>

<dependencies>
- Moduler ska bero på abstraktioner
- vga.c känner inte till player.c
- player.c anropar vga_draw_rect(), inte raw VGA
- Cirkulära beroenden är förbjudna
</dependencies>

<game_loop>
while (!quit) {
    input_update();     /* Läs tangentbord */
    player_update();    /* Uppdatera spellogik */
    level_update();     /* Uppdatera värld */
    
    vga_clear();        /* Rensa buffer */
    level_draw();       /* Rita värld */
    player_draw();      /* Rita spelare */
    
    vga_vsync();        /* Vänta på vsync */
    vga_flip();         /* Kopiera till skärm */
}
</game_loop>
