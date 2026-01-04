/*
 * level.h - Nivå och plattformar
 */
#ifndef LEVEL_H
#define LEVEL_H

void level_init(void);
void level_draw(void);
int level_check_collision(int x, int y, int w, int h);
int level_get_platform_top(int index);

#endif
