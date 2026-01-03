/*
 * player.c - Spelarlogik och fysik
 */
#include "player.h"
#include "level.h"
#include "vga.h"
#include "types.h"

#define GRAVITY 1
#define JUMP_FORCE -8
#define MOVE_SPEED 3

void player_init(Player *p, int x, int y) {
    p->x = x;
    p->y = y;
    p->vy = 0;
    p->on_ground = 0;
}

void player_update(Player *p, int left, int right, int jump) {
    int new_x, new_y;
    int collision;
    
    /* Horisontell rörelse */
    new_x = p->x;
    if (left) new_x -= MOVE_SPEED;
    if (right) new_x += MOVE_SPEED;
    
    /* Håll spelaren inom skärmen */
    if (new_x < 0) new_x = 0;
    if (new_x > SCREEN_W - PLAYER_W) new_x = SCREEN_W - PLAYER_W;
    
    /* Kolla horisontell kollision */
    collision = level_check_collision(new_x, p->y, PLAYER_W, PLAYER_H);
    if (collision < 0) {
        p->x = new_x;
    }
    
    /* Hopp */
    if (jump && p->on_ground) {
        p->vy = JUMP_FORCE;
        p->on_ground = 0;
    }
    
    /* Gravitation */
    p->vy += GRAVITY;
    if (p->vy > 10) p->vy = 10;
    
    /* Vertikal rörelse */
    new_y = p->y + p->vy;
    
    /* Kolla landning på plattformar */
    p->on_ground = 0;
    collision = level_check_collision(p->x, new_y, PLAYER_W, PLAYER_H);
    
    if (collision >= 0 && p->vy > 0) {
        /* Landa på plattform */
        p->y = level_get_platform_top(collision) - PLAYER_H;
        p->vy = 0;
        p->on_ground = 1;
    } else if (collision >= 0 && p->vy < 0) {
        /* Slog i huvudet */
        p->vy = 0;
    } else {
        p->y = new_y;
    }
    
    /* Respawn om spelaren faller ut */
    if (p->y > SCREEN_H) {
        player_init(p, 50, 150);
    }
}

void player_draw(Player *p) {
    /* Enkel rektangel-gubbe */
    vga_draw_rect(p->x, p->y, PLAYER_W, PLAYER_H, 14);      /* Gul kropp */
    vga_draw_rect(p->x + 2, p->y + 2, 8, 6, 4);             /* Rött huvud */
}
