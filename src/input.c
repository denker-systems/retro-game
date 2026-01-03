/*
 * input.c - Tangentbordshantering via DOS
 * 
 * Använder kbhit() och getch() från conio.h
 * Piltangenter skickar två bytes: 0x00 eller 0xE0 följt av scankod
 */
#include <conio.h>
#include "input.h"

void input_update(InputState *state) {
    unsigned char key;
    
    /* Nollställ one-shot inputs */
    state->jump = 0;
    
    while (kbhit()) {
        key = getch();
        
        if (key == 0 || key == 0xE0) {
            /* Extended key - läs scankod */
            key = getch();
            switch (key) {
                case 75: state->left = 1; break;   /* Vänsterpil */
                case 77: state->right = 1; break;  /* Högerpil */
            }
        } else {
            switch (key) {
                case 27: state->quit = 1; break;   /* ESC */
                case 32: state->jump = 1; break;   /* Mellanslag */
            }
        }
    }
}
