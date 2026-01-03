/*
 * level.c - Nivådata och kollisioner
 */
#include "level.h"
#include "vga.h"

/* Plattformar: x, y, bredd, höjd */
static int platforms[][4] = {
    {0, 180, 320, 20},   /* Marken */
    {50, 140, 60, 10},   /* Plattform 1 */
    {150, 110, 60, 10},  /* Plattform 2 */
    {240, 80, 60, 10},   /* Plattform 3 */
    {100, 50, 80, 10}    /* Toppplattform */
};

#define NUM_PLATFORMS 5

void level_init(void) {
    /* Framtida: ladda nivå från fil */
}

void level_draw(void) {
    int i;
    for (i = 0; i < NUM_PLATFORMS; i++) {
        /* Marken är brun (6), plattformar gröna (2) */
        vga_draw_rect(
            platforms[i][0], 
            platforms[i][1],
            platforms[i][2], 
            platforms[i][3],
            i == 0 ? 6 : 2
        );
    }
}

int level_check_collision(int x, int y, int w, int h) {
    int i;
    for (i = 0; i < NUM_PLATFORMS; i++) {
        int px = platforms[i][0];
        int py = platforms[i][1];
        int pw = platforms[i][2];
        int ph = platforms[i][3];
        
        /* AABB kollisionsdetektion */
        if (x < px + pw && x + w > px && 
            y < py + ph && y + h > py) {
            return i;
        }
    }
    return -1;
}

int level_get_platform_top(int index) {
    if (index >= 0 && index < NUM_PLATFORMS) {
        return platforms[index][1];
    }
    return SCREEN_H;
}
