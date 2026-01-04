/*
 * input.c - Tangentbordshantering via DOS
 * 
 * SYFTE:
 *   Denna modul läser tangentbordsinput och uppdaterar spelets
 *   InputState-struktur. Resten av spelet behöver inte veta hur
 *   tangentbordet fungerar - de kollar bara state->left, etc.
 * 
 * HÅRDVARA - PC-tangentbordet:
 *   PC-tangentbordet är anslutet via en Intel 8042-kontroller.
 *   När du trycker på en tangent händer följande:
 *   
 *   1. Tangentbordet skickar en "scan code" till 8042-kontrollern
 *   2. 8042 genererar en hardware interrupt (IRQ 1 -> INT 9)
 *   3. BIOS interrupt handler läser scan code och lagrar den
 *   4. DOS/BIOS tillhandahåller funktioner för att läsa tangenter
 * 
 * SCAN CODES vs ASCII:
 *   - ASCII-tangenter (A-Z, 0-9, etc.) returnerar sitt ASCII-värde
 *   - Specialtangenter (pilar, F1-F12, etc.) returnerar TVÅ bytes:
 *     Första byte: 0x00 eller 0xE0 (markerar "extended key")
 *     Andra byte: Scan code som identifierar tangenten
 * 
 * SCAN CODES FÖR PILTANGENTER:
 *   Vänster: 75 (0x4B)
 *   Höger:   77 (0x4D)
 *   Upp:     72 (0x48)
 *   Ner:     80 (0x50)
 * 
 * BEROENDEN:
 *   - conio.h: kbhit() och getch() för tangentbordsläsning
 *   - input.h: InputState struktur-definition
 */

#include <conio.h>   /* kbhit(), getch() - DOS console I/O */
#include "input.h"

/*
 * input_update - Läser tangentbord och uppdaterar InputState
 * 
 * PARAMETRAR:
 *   state - Pekare till InputState som ska uppdateras
 *           Fälten left, right, jump, quit sätts baserat på input
 * 
 * FUNGERAR SÅ HÄR:
 *   1. Nollställer "one-shot" inputs (jump)
 *   2. Loopar genom alla väntande tangenter
 *   3. Hanterar både vanliga och extended tangenter
 *   4. Sätter motsvarande flaggor i state
 * 
 * ONE-SHOT vs CONTINUOUS:
 *   - "jump" nollställs varje frame - du måste släppa och trycka igen
 *   - "left/right" hålls kvar tills nästa uppdatering
 *   - "quit" sätts permanent när ESC trycks
 * 
 * VARFÖR WHILE-LOOP?
 *   Flera tangenter kan ha tryckts sedan förra frame.
 *   Vi läser ALLA väntande tangenter, inte bara den första.
 */
void input_update(InputState *state) {
    /*
     * key - Variabel för att lagra läst tangentvärde
     * 
     * unsigned char (0-255) eftersom:
     * - ASCII-värden är 0-127
     * - Extended key markers är 0x00 eller 0xE0
     * - Scan codes är 0-255
     */
    unsigned char key;
    
    /*
     * Nollställ "one-shot" inputs
     * 
     * Jump ska bara vara aktiv EN frame när tangenten trycks.
     * Om vi inte nollställer kommer spelaren hoppa varje frame
     * så länge mellanslag hålls nere (inte önskat beteende).
     */
    state->up = 0;
    state->down = 0;
    state->jump = 0;
    state->enter = 0;
    state->pause = 0;
    
    /*
     * Läs alla väntande tangenter
     * 
     * kbhit() - "Keyboard Hit"
     *   Returnerar icke-noll om en tangent väntar i buffern.
     *   Blockerar INTE - returnerar omedelbart.
     *   
     * getch() - "Get Character"
     *   Läser nästa tecken från tangentbordsbuffern.
     *   Blockerar tills en tangent är tillgänglig (men vi
     *   anropar bara efter kbhit() så det blockerar aldrig).
     *   Ekar INTE tecknet till skärmen.
     */
    while (kbhit()) {
        key = getch();  /* Läs första/enda byte */
        
        /*
         * Kontrollera om det är en "extended key"
         * 
         * 0x00: Original IBM PC extended key marker
         * 0xE0: AT-tangentbord extended key marker (nyare)
         * 
         * Om vi får 0 eller 0xE0 måste vi läsa EN TILL byte
         * för att få den faktiska scan code.
         */
        if (key == 0 || key == 0xE0) {
            /*
             * Extended key - läs scan code
             * 
             * Andra anropet till getch() ger oss scan code
             * som identifierar vilken specialtangent det är.
             */
            key = getch();
            
            switch (key) {
                /*
                 * Scan code 75 (0x4B) = Vänsterpil
                 * 
                 * Sätter left-flaggan som player.c kollar.
                 * Vi sätter till 1 men nollställer inte -
                 * det görs av main.c efter player_update().
                 */
                case 75: state->left = 1; break;
                case 77: state->right = 1; break;
                case 72: state->up = 1; break;    /* Upp */
                case 80: state->down = 1; break;  /* Ner */
            }
        } else {
            /*
             * Vanlig ASCII-tangent
             * 
             * Värdet i 'key' är direkt ASCII-koden.
             */
            switch (key) {
                /*
                 * ASCII 27 = ESC (Escape)
                 * 
                 * Sätter quit-flaggan som avslutar spelet.
                 * Denna nollställs ALDRIG - när den är satt
                 * avslutas huvudloopen.
                 */
                case 27: state->quit = 1; break;
                
                /*
                 * ASCII 32 = Mellanslag (Space)
                 * 
                 * Sätter jump-flaggan för ett hopp.
                 * Nollställs i början av NÄSTA frame,
                 * så hoppet registreras bara en gång per tryck.
                 */
                case 32: state->jump = 1; break;
                
                /*
                 * ASCII 112 = 'p' (Pause)
                 * ASCII 80 = 'P' (Pause)
                 */
                case 'p':
                case 'P': state->pause = 1; break;
                
                case 13: state->enter = 1; break;  /* Enter */
            }
        }
    }
}
