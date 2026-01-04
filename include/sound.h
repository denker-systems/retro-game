/*
 * sound.h - Ljudsystem (Sound Blaster + PC Speaker fallback)
 * 
 * ARKITEKTUR:
 *   Försöker använda Sound Blaster om tillgänglig.
 *   Faller tillbaka på PC Speaker annars.
 *   
 * SOUND BLASTER:
 *   Detekteras via BLASTER miljövariabel (t.ex. "A220 I5 D1")
 *   - A = Base port (vanligtvis 220h)
 *   - I = IRQ (vanligtvis 5 eller 7)
 *   - D = DMA kanal (vanligtvis 1)
 *   
 * PC SPEAKER:
 *   Fallback - finns på alla IBM PC.
 *   Timer 2 (PIT) genererar fyrkantvåg.
 */

#ifndef SOUND_H
#define SOUND_H

/* Ljudeffekt-ID */
#define SFX_JUMP      0
#define SFX_MENU_MOVE 1
#define SFX_MENU_SELECT 2
#define SFX_LAND      3

/* Sound Blaster-status */
extern int sb_detected;      /* 1 om SB hittades */
extern int sb_base_port;     /* Base I/O port (220h) */
extern int sb_irq;           /* IRQ nummer */
extern int sb_dma;           /* DMA kanal */

/*
 * sound_init - Initierar ljudsystemet
 * 
 * Detekterar tillgänglig hårdvara (PC Speaker alltid, SB om finns).
 */
void sound_init(void);

/*
 * sound_close - Stänger av ljudsystemet
 * 
 * Viktigt att anropa vid avslut för att tysta speaker.
 */
void sound_close(void);

/*
 * sound_play - Spelar en ljudeffekt
 * 
 * Parametrar:
 *   sfx_id - Vilken effekt (SFX_JUMP, etc.)
 */
void sound_play(int sfx_id);

/*
 * sound_beep - Spelar en ton på PC Speaker
 * 
 * Parametrar:
 *   frequency - Frekvens i Hz (0 = tyst)
 *   duration_ms - Längd i millisekunder
 */
void sound_beep(int frequency, int duration_ms);

/*
 * sound_off - Stänger av PC Speaker omedelbart
 */
void sound_off(void);

/*
 * sound_jingle - Spelar en kort melodi
 * 
 * Parametrar:
 *   jingle_id - Vilken melodi (JINGLE_INTRO, etc.)
 */
#define JINGLE_INTRO  0
#define JINGLE_WIN    1

void sound_jingle(int jingle_id);

/*
 * sb_play_sample - Spelar 8-bit PCM via Sound Blaster
 * 
 * Parametrar:
 *   data   - Pekare till ljuddata (8-bit unsigned PCM)
 *   length - Antal bytes
 *   rate   - Sample rate (11025, 22050, etc.)
 */
void sb_play_sample(unsigned char far *data, unsigned int length, unsigned int rate);

/*
 * Musik-streaming (för längre ljudfiler)
 * 
 * Använder double-buffering för att spela stora WAV-filer
 * utan att ladda hela filen i minnet.
 */

/* Starta musikuppspelning från WAV-fil */
int music_play(const char *filename, int loop);

/* Stoppa musik */
void music_stop(void);

/* Uppdatera musik-streaming (kalla varje frame) */
void music_update(void);

/* Kontrollera om musik spelas */
int music_playing(void);

#endif
