/*
 * game.c - Spelinstans och save/load
 * 
 * DOS FILE I/O:
 *   I DOS använder vi standard C-biblioteket för filhantering:
 *   - fopen()  - Öppna fil
 *   - fwrite() - Skriv binärdata
 *   - fread()  - Läs binärdata
 *   - fclose() - Stäng fil
 *   
 *   "wb" = Write Binary (skriv binärt)
 *   "rb" = Read Binary (läs binärt)
 *   
 *   Binärläge är viktigt! Utan det kan DOS ändra newline-tecken.
 */

#include <stdio.h>
#include "game.h"
#include "player.h"

/*
 * game_new - Skapar ny spelinstans
 */
void game_new(GameState *state, Player *player)
{
    /* Återställ spelaren till startposition */
    player_init(player, 50, 150);
    
    /* Nollställ spelstatistik */
    state->score = 0;
    state->level = 1;
    state->magic = SAVE_MAGIC;
    
    /* Synka state med player */
    state->player_x = player->x;
    state->player_y = player->y;
    state->player_vy = player->vy;
    state->player_on_ground = player->on_ground;
}

/*
 * game_save - Sparar spelet till fil
 */
int game_save(GameState *state, Player *player)
{
    FILE *f;
    
    /* Kopiera spelardata till state */
    state->player_x = player->x;
    state->player_y = player->y;
    state->player_vy = player->vy;
    state->player_on_ground = player->on_ground;
    state->magic = SAVE_MAGIC;
    
    /* Öppna fil för binär skrivning */
    f = fopen(SAVE_FILENAME, "wb");
    if (f == NULL) {
        return 0;  /* Kunde inte öppna filen */
    }
    
    /* Skriv hela structen på en gång */
    if (fwrite(state, sizeof(GameState), 1, f) != 1) {
        fclose(f);
        return 0;  /* Skrivning misslyckades */
    }
    
    fclose(f);
    return 1;  /* Lyckades! */
}

/*
 * game_load - Laddar spel från fil
 */
int game_load(GameState *state, Player *player)
{
    FILE *f;
    
    /* Öppna fil för binär läsning */
    f = fopen(SAVE_FILENAME, "rb");
    if (f == NULL) {
        return 0;  /* Fil finns inte */
    }
    
    /* Läs hela structen */
    if (fread(state, sizeof(GameState), 1, f) != 1) {
        fclose(f);
        return 0;  /* Läsning misslyckades */
    }
    
    fclose(f);
    
    /* Verifiera magiskt nummer */
    if (state->magic != SAVE_MAGIC) {
        return 0;  /* Ogiltig eller korrupt fil */
    }
    
    /* Återställ spelardata från state */
    player->x = state->player_x;
    player->y = state->player_y;
    player->vy = state->player_vy;
    player->on_ground = state->player_on_ground;
    
    return 1;  /* Lyckades! */
}

/*
 * game_exists - Kontrollerar om sparfil finns
 */
int game_exists(void)
{
    FILE *f;
    
    f = fopen(SAVE_FILENAME, "rb");
    if (f == NULL) {
        return 0;
    }
    
    fclose(f);
    return 1;
}
