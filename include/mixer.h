/*
 * mixer.h - Software Audio Mixer för DOS
 * 
 * SYFTE:
 *   Mixar musik och ljudeffekter till en enda audio-stream.
 *   Möjliggör samtidig uppspelning av musik + flera SFX.
 */

#ifndef MIXER_H
#define MIXER_H

/*
 * mixer_init - Initierar mixer-systemet
 * 
 * Detekterar Sound Blaster, allokerar buffertar, installerar IRQ.
 * RETURNERAR: 1 om OK, 0 om fel
 */
int mixer_init(void);

/*
 * mixer_close - Stänger mixer-systemet
 * 
 * Stoppar uppspelning, frigör minne, återställer IRQ.
 */
void mixer_close(void);

/*
 * mixer_start - Startar audio-streaming
 * 
 * Börjar kontinuerlig uppspelning av mixad audio.
 */
void mixer_start(void);

/*
 * mixer_stop - Stoppar audio-streaming
 */
void mixer_stop(void);

/*
 * mixer_update - Uppdaterar mixer
 * 
 * VIKTIGT: Anropa varje frame!
 * Fyller buffertar och hanterar buffert-byte.
 */
void mixer_update(void);

/*
 * mixer_play_music - Spelar musik från WAV-fil
 * 
 * PARAMETRAR:
 *   filename - Sökväg till WAV-fil (8-bit unsigned PCM, 11025 Hz)
 *   loop     - 1 för loop, 0 för engångs
 *   
 * RETURNERAR: 1 om OK, 0 om fel
 */
int mixer_play_music(const char *filename, int loop);

/*
 * mixer_stop_music - Stoppar musik
 */
void mixer_stop_music(void);

/*
 * mixer_play_sfx - Spelar ljudeffekt
 * 
 * PARAMETRAR:
 *   data   - Pekare till ljuddata (8-bit unsigned PCM)
 *   length - Längd i bytes
 *   loop   - 1 för loop, 0 för engångs
 *   
 * RETURNERAR: Kanal-nummer (0-3) eller -1 om ingen ledig kanal
 */
int mixer_play_sfx(unsigned char far *data, unsigned long length, int loop);

/*
 * mixer_stop_sfx - Stoppar ljudeffekt på given kanal
 */
void mixer_stop_sfx(int channel);

#endif /* MIXER_H */
