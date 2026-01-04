/*
 * game.h - Spelinstans och save/load
 * 
 * DOS-HISTORIK:
 *   I DOS-eran sparades speldata oftast som enkla binärfiler.
 *   Man skrev struct:en direkt till disk med fwrite() och
 *   läste tillbaka med fread(). Enkelt och effektivt!
 *   
 *   Spelfiler hade ofta extensioner som .SAV, .DAT eller .GAM
 */

#ifndef GAME_H
#define GAME_H

#include "player.h"

/*
 * GameState - Hela spelets tillstånd
 * 
 * Allt som behöver sparas för att återställa spelet.
 * Strukturen skrivs direkt till fil som binärdata.
 */
typedef struct {
    /* Spelardata */
    int player_x;
    int player_y;
    int player_vy;
    int player_on_ground;
    
    /* Spelstatistik */
    int score;
    int level;
    
    /* Magiskt nummer för att verifiera filen */
    unsigned int magic;
} GameState;

/* Magiskt nummer för att identifiera våra save-filer */
#define SAVE_MAGIC 0x5245  /* "RE" i ASCII (Retro) */

/* Filnamn för sparfilen */
#define SAVE_FILENAME "GAME.SAV"

/*
 * game_new - Skapar ny spelinstans
 */
void game_new(GameState *state, Player *player);

/*
 * game_save - Sparar spelet till fil
 * 
 * Returnerar:
 *   1 om sparning lyckades
 *   0 om det misslyckades
 */
int game_save(GameState *state, Player *player);

/*
 * game_load - Laddar spel från fil
 * 
 * Returnerar:
 *   1 om laddning lyckades
 *   0 om det misslyckades (fil saknas eller korrupt)
 */
int game_load(GameState *state, Player *player);

/*
 * game_exists - Kontrollerar om sparfil finns
 * 
 * Returnerar:
 *   1 om GAME.SAV finns
 *   0 om den inte finns
 */
int game_exists(void);

#endif
