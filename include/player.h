/*
 * player.h - Spelarlogik
 */
#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_W 12
#define PLAYER_H 16

typedef struct {
    int x, y;
    int vy;
    int on_ground;
} Player;

void player_init(Player *p, int x, int y);
void player_update(Player *p, int left, int right, int jump);
void player_draw(Player *p);

#endif
