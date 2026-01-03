/*
 * main.c - Huvudprogram och spelloop
 * 
 * RETRO DOS PLATFORMER
 * Kompilera med Open Watcom: wcl -0 -ms src\*.c -fe=game.exe
 */
#include <conio.h>
#include "types.h"
#include "vga.h"
#include "input.h"
#include "player.h"
#include "level.h"

int main(void) {
    Player player;
    InputState input = {0, 0, 0, 0};
    
    /* Initiera moduler */
    vga_init();
    level_init();
    player_init(&player, 50, 150);
    
    /* Huvudloop */
    while (!input.quit) {
        /* Läs input */
        input_update(&input);
        
        /* Uppdatera spellogik */
        player_update(&player, input.left, input.right, input.jump);
        
        /* Nollställ rörelsestate (hålls inte mellan frames) */
        input.left = 0;
        input.right = 0;
        
        /* Rita */
        vga_clear(1);           /* Mörkblå himmel */
        level_draw();
        player_draw(&player);
        
        /* Visa på skärm */
        vga_vsync();
        vga_flip();
    }
    
    /* Städa upp */
    vga_close();
    cputs("Tack for att du spelade!\r\n");
    
    return 0;
}
