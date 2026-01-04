/*
 * menu.h - Startmeny och titelskärm
 */

#ifndef MENU_H
#define MENU_H

/* Huvudmeny-val */
#define MENU_NEW      0
#define MENU_CONTINUE 1
#define MENU_QUIT     2

/* Pausmeny-val */
#define PAUSE_RESUME 0
#define PAUSE_SAVE   1
#define PAUSE_MENU   2

/*
 * menu_show - Visar titelskärm och meny
 * 
 * Parametrar:
 *   has_save - 1 om sparfil finns (visar CONTINUE)
 * 
 * Returnerar:
 *   MENU_NEW, MENU_CONTINUE eller MENU_QUIT
 */
int menu_show(int has_save);

/*
 * pause_show - Visar pausmeny
 * 
 * Returnerar:
 *   PAUSE_RESUME om spelaren valde "Fortsätt"
 *   PAUSE_QUIT om spelaren valde "Avsluta"
 */
int pause_show(void);

#endif
