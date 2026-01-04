/*
 * menu.c - Startmeny och titelskärm
 * 
 * SYFTE:
 *   Visar en enkel titelskärm med spelets namn och en meny
 *   där spelaren kan välja att starta eller avsluta.
 * 
 * RENDERING:
 *   Eftersom vi inte har font-rendering ritar vi bokstäver
 *   som enkla pixel-mönster (bitmap font).
 */

#include <conio.h>
#include "menu.h"
#include "vga.h"
#include "input.h"
#include "sound.h"

/* Färger */
#define COLOR_BG      1   /* Mörkblå bakgrund */
#define COLOR_TITLE  14   /* Gul titel */
#define COLOR_TEXT   15   /* Vit text */
#define COLOR_SELECT 10   /* Ljusgrön markering */

/*
 * draw_char - Ritar en enkel 5x7 pixel-bokstav
 * 
 * Vi har bara de bokstäver vi behöver för menyn.
 * Varje bokstav är definierad som 7 rader med 5 bitar vardera.
 */
static void draw_char(int x, int y, char c, unsigned char color)
{
    /* Enkel bitmap font - bara de tecken vi behöver */
    static const unsigned char font_R[] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11};
    static const unsigned char font_E[] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F};
    static const unsigned char font_T[] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04};
    static const unsigned char font_O[] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const unsigned char font_G[] = {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E};
    static const unsigned char font_A[] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11};
    static const unsigned char font_M[] = {0x11,0x1B,0x15,0x11,0x11,0x11,0x11};
    static const unsigned char font_P[] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10};
    static const unsigned char font_L[] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F};
    static const unsigned char font_Y[] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04};
    static const unsigned char font_Q[] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D};
    static const unsigned char font_U[] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const unsigned char font_I[] = {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E};
    static const unsigned char font_S[] = {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E};
    static const unsigned char font_D[] = {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C};
    static const unsigned char font_N[] = {0x11,0x19,0x15,0x13,0x11,0x11,0x11};
    static const unsigned char font_K[] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11};
    static const unsigned char font_V[] = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04};
    static const unsigned char font_W[] = {0x11,0x11,0x11,0x15,0x15,0x1B,0x11};
    static const unsigned char font_C[] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E};
    static const unsigned char font_gt[]= {0x10,0x08,0x04,0x02,0x04,0x08,0x10};
    
    const unsigned char *font = 0;
    int row, col;
    
    switch(c) {
        case 'R': font = font_R; break;
        case 'E': font = font_E; break;
        case 'T': font = font_T; break;
        case 'O': font = font_O; break;
        case 'G': font = font_G; break;
        case 'A': font = font_A; break;
        case 'M': font = font_M; break;
        case 'P': font = font_P; break;
        case 'L': font = font_L; break;
        case 'Y': font = font_Y; break;
        case 'Q': font = font_Q; break;
        case 'U': font = font_U; break;
        case 'I': font = font_I; break;
        case 'S': font = font_S; break;
        case 'D': font = font_D; break;
        case 'N': font = font_N; break;
        case 'K': font = font_K; break;
        case 'V': font = font_V; break;
        case 'W': font = font_W; break;
        case 'C': font = font_C; break;
        case '>': font = font_gt; break;
        default: return;
    }
    
    for (row = 0; row < 7; row++) {
        for (col = 0; col < 5; col++) {
            if (font[row] & (0x10 >> col)) {
                vga_draw_rect(x + col*2, y + row*2, 2, 2, color);
            }
        }
    }
}

/*
 * draw_text - Ritar en textsträng
 */
static void draw_text(int x, int y, const char *text, unsigned char color)
{
    while (*text) {
        if (*text != ' ') {
            draw_char(x, y, *text, color);
        }
        x += 12;
        text++;
    }
}

/*
 * clear_input - Rensar tangentbordsbuffern helt
 */
static void clear_input(void)
{
    while (kbhit()) getch();
}

/*
 * wait_key_release - Väntar lite så inte samma knapptryck registreras flera gånger
 */
static void wait_key_release(void)
{
    int i;
    
    clear_input();
    
    /* Vänta några frames */
    for (i = 0; i < 3; i++) {
        vga_vsync();
    }
    clear_input();
}

/*
 * menu_show - Visar titelskärm och meny
 */
int menu_show(int has_save)
{
    int selection = 0;
    int max_sel = has_save ? 2 : 1;
    InputState input = {0,0,0,0,0,0,0,0};
    int y_pos;
    
    /* Starta menymusik (loopa) */
    music_play("SOUNDS\\MAINMENU.WAV", 1);
    
    wait_key_release();
    
    while (1) {
        input_update(&input);
        
        /* Uppdatera musik-streaming */
        music_update();
        
        if (input.enter) {
            sound_play(SFX_MENU_SELECT);
            music_stop();
            if (selection == 0) return MENU_NEW;
            if (has_save && selection == 1) return MENU_CONTINUE;
            return MENU_QUIT;
        }
        if (input.quit) {
            music_stop();
            return MENU_QUIT;
        }
        
        if (input.up) {
            selection = (selection > 0) ? selection - 1 : max_sel;
            sound_play(SFX_MENU_MOVE);
            wait_key_release();
        }
        if (input.down) {
            selection = (selection < max_sel) ? selection + 1 : 0;
            sound_play(SFX_MENU_MOVE);
            wait_key_release();
        }
        
        /* Rita meny */
        vga_clear(COLOR_BG);
        
        draw_text(80, 40, "RETRO GAME", COLOR_TITLE);
        draw_text(95, 70, "DOS EDITION", COLOR_TEXT);
        
        /* Menyval */
        y_pos = 110;
        draw_text(100, y_pos, "NEW GAME", selection == 0 ? COLOR_SELECT : COLOR_TEXT);
        if (selection == 0) draw_text(80, y_pos, ">", COLOR_SELECT);
        
        if (has_save) {
            y_pos += 20;
            draw_text(100, y_pos, "CONTINUE", selection == 1 ? COLOR_SELECT : COLOR_TEXT);
            if (selection == 1) draw_text(80, y_pos, ">", COLOR_SELECT);
        }
        
        y_pos += 20;
        draw_text(100, y_pos, "QUIT", selection == max_sel ? COLOR_SELECT : COLOR_TEXT);
        if (selection == max_sel) draw_text(80, y_pos, ">", COLOR_SELECT);
        
        draw_text(60, 180, "KEYS TO SELECT", 7);
        
        vga_vsync();
        vga_flip();
    }
}

/*
 * pause_show - Visar pausmeny under spelet
 * 
 * Returnerar:
 *   PAUSE_RESUME, PAUSE_SAVE eller PAUSE_MENU
 */
int pause_show(void)
{
    int selection = 0;
    InputState input = {0,0,0,0,0,0,0,0};
    
    wait_key_release();
    
    while (1) {
        input_update(&input);
        
        if (input.pause) {
            return PAUSE_RESUME;
        }
        
        if (input.enter) {
            sound_play(SFX_MENU_SELECT);
            return selection;
        }
        
        if (input.up) {
            selection = (selection > 0) ? selection - 1 : 2;
            sound_play(SFX_MENU_MOVE);
            wait_key_release();
        }
        if (input.down) {
            selection = (selection < 2) ? selection + 1 : 0;
            sound_play(SFX_MENU_MOVE);
            wait_key_release();
        }
        
        /* Rita pausmeny */
        vga_clear(0);
        
        draw_text(115, 50, "PAUSED", COLOR_TITLE);
        
        /* Menyval */
        draw_text(110, 90, "RESUME", selection == 0 ? COLOR_SELECT : COLOR_TEXT);
        if (selection == 0) draw_text(90, 90, ">", COLOR_SELECT);
        
        draw_text(120, 115, "SAVE", selection == 1 ? COLOR_SELECT : COLOR_TEXT);
        if (selection == 1) draw_text(100, 115, ">", COLOR_SELECT);
        
        draw_text(120, 140, "MENU", selection == 2 ? COLOR_SELECT : COLOR_TEXT);
        if (selection == 2) draw_text(100, 140, ">", COLOR_SELECT);
        
        draw_text(70, 175, "P TO RESUME", 7);
        
        vga_vsync();
        vga_flip();
    }
}
