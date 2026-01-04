/*
 * main.c - Huvudprogram och spelloop
 * 
 * RETRO DOS PLATFORMER
 * Kompilera med Open Watcom: wcl -0 -ms src\*.c -fe=game.exe
 * 
 * SYFTE:
 *   Detta är programmets startpunkt (entry point).
 *   Här initieras alla moduler och huvudloopen körs.
 *   main.c är "dirigenten" som koordinerar alla andra moduler.
 * 
 * SPELLOOP-ARKITEKTUR:
 *   Alla spel har en "game loop" - en oändlig loop som kör
 *   tills spelaren avslutar. Varje varv i loopen kallas en "frame".
 *   
 *   Standard spelloop:
 *   
 *   while (spelar) {
 *       1. INPUT:  Läs spelarens knapptryckningar
 *       2. UPDATE: Uppdatera spellogik (fysik, AI, kollisioner)
 *       3. RENDER: Rita allt på skärmen
 *   }
 *   
 *   Denna ordning är viktig:
 *   - Input FÖRST så att update har färsk data
 *   - Update FÖRE render så att vi ritar rätt tillstånd
 *   - Render SIST så att spelaren ser resultatet
 * 
 * FRAME RATE:
 *   Vår loop synkroniseras med VGA:s vertical retrace (~70 Hz).
 *   Det betyder att spelet kör med ~70 frames per sekund.
 *   
 *   Fördelar:
 *   - Ingen screen tearing (bild ritas mellan skärmuppdateringar)
 *   - Konsekvent hastighet på alla datorer (begränsas av skärmen)
 *   
 *   Nackdelar:
 *   - Spelet kan inte köra snabbare än 70 FPS
 *   - På långsamma datorer kan det bli "frame drops"
 * 
 * BEROENDEN:
 *   - conio.h: cputs() för textutskrift
 *   - types.h: Grundläggande typdefinitioner
 *   - vga.h: Grafikinitiering och rendering
 *   - input.h: Tangentbordsläsning
 *   - player.h: Spelarlogik
 *   - level.h: Nivådata och rendering
 */

#include <conio.h>   /* cputs() för att skriva text vid avslut */
#include "types.h"   /* byte, SCREEN_W, SCREEN_H */
#include "vga.h"     /* vga_init(), vga_close(), vga_clear(), etc. */
#include "input.h"   /* InputState, input_update() */
#include "player.h"  /* Player, player_init(), player_update(), player_draw() */
#include "level.h"   /* level_init(), level_draw() */

/*
 * main - Programmets startpunkt
 * 
 * RETURNERAR:
 *   0 vid normal avslutning (konvention i C)
 * 
 * FUNGERAR SÅ HÄR:
 *   1. Initierar alla spelmoduler
 *   2. Kör huvudloopen tills spelaren trycker ESC
 *   3. Städar upp och avslutar
 */
int main(void) {
    /*
     * Player player - Spelarens tillstånd
     * 
     * Denna struct innehåller allt om spelaren:
     * position (x, y), hastighet (vy), och om de står på marken.
     * 
     * Vi deklarerar den på stacken (inte heap) eftersom:
     * - Den är liten (några int-variabler)
     * - Den behövs under hela programmets livstid
     * - Stack-allokering är snabbare och enklare
     */
    Player player;
    
    /*
     * InputState input - Tangentbordstillstånd
     * 
     * Struct som håller reda på vilka tangenter som är nedtryckta.
     * Initierad till {0, 0, 0, 0} = alla falska (ingen knapp nedtryckt).
     * 
     * Fält: left, right, jump, quit
     */
    InputState input = {0, 0, 0, 0};
    
    /* ========== INITIERING ========== */
    
    /*
     * Initiera VGA-grafik
     * 
     * Detta måste göras FÖRST eftersom:
     * 1. Allokerar dubbelbuffern (behövs för ritning)
     * 2. Sätter skärmen i Mode 13h (grafikläge)
     * 
     * Efter detta anrop är skärmen i grafikläge och
     * normal textutskrift fungerar inte längre!
     */
    vga_init();
    
    /*
     * Initiera nivån
     * 
     * Just nu gör detta ingenting (nivådata är hårdkodad),
     * men i ett större spel skulle detta ladda nivåfilen.
     */
    level_init();
    
    /*
     * Initiera spelaren
     * 
     * Placerar spelaren på position (50, 150).
     * Y=150 är lite ovanför marken (Y=180), så spelaren
     * kommer att falla ner och landa på första frame.
     * 
     * &player = "adressen till player-variabeln"
     * Funktionen behöver en pekare för att kunna modifiera player.
     */
    player_init(&player, 50, 150);
    
    /* ========== HUVUDLOOP ========== */
    
    /*
     * Spelloopen - hjärtat av spelet
     * 
     * Körs ~70 gånger per sekund (synkat med VGA vsync).
     * Avslutas när input.quit blir sant (ESC trycks).
     * 
     * !input.quit = "så länge quit INTE är sant"
     */
    while (!input.quit) {
        
        /* ----- INPUT PHASE ----- */
        
        /*
         * Läs tangentbord
         * 
         * input_update() kollar vilka tangenter som tryckts
         * och uppdaterar input-structen.
         * 
         * &input = pekare så funktionen kan modifiera structen
         */
        input_update(&input);
        
        /* ----- UPDATE PHASE ----- */
        
        /*
         * Uppdatera spelarens fysik och position
         * 
         * Skickar in input-flaggorna så spelaren vet
         * vilka tangenter som är nedtryckta.
         * 
         * Ordningen på argumenten:
         * 1. &player - spelardatan att uppdatera
         * 2. input.left - 1 om vänsterpil, annars 0
         * 3. input.right - 1 om högerpil, annars 0
         * 4. input.jump - 1 om mellanslag, annars 0
         */
        player_update(&player, input.left, input.right, input.jump);
        
        /*
         * Nollställ rörelse-input
         * 
         * left och right nollställs här efter att de använts.
         * Detta gör att spelaren slutar röra sig om tangenten släpps.
         * 
         * (jump nollställs av input_update() nästa frame)
         * (quit nollställs ALDRIG - när den är satt avslutar vi)
         */
        input.left = 0;
        input.right = 0;
        
        /* ----- RENDER PHASE ----- */
        
        /*
         * Rensa skärmen
         * 
         * Fyller hela dubbelbuffern med en färg.
         * Färg 1 = Mörkblå (vår "himmel")
         * 
         * Vi måste rensa varje frame eftersom vi ritar
         * spelaren på ny position - den gamla positionen
         * måste "raderas".
         */
        vga_clear(1);
        
        /*
         * Rita nivån (plattformarna)
         * 
         * Ritas FÖRE spelaren så att spelaren visas
         * "ovanpå" plattformarna (korrekt Z-ordning).
         */
        level_draw();
        
        /*
         * Rita spelaren
         * 
         * Ritas SIST av spelobjekten så att spelaren
         * alltid syns (inte döljs av plattformar).
         */
        player_draw(&player);
        
        /* ----- PRESENT PHASE ----- */
        
        /*
         * Vänta på vertical retrace
         * 
         * Detta synkroniserar vår rendering med skärmens
         * uppdateringsfrekvens. Förhindrar "tearing".
         * 
         * Vi väntar INNAN vi kopierar till VGA-minnet
         * så att kopieringen sker under retrace-perioden.
         */
        vga_vsync();
        
        /*
         * Kopiera dubbelbuffern till skärmen
         * 
         * Nu när vi väntat på vsync är det säkert att
         * kopiera. Hela buffern (64000 bytes) kopieras
         * till VGA-minnet på en gång.
         * 
         * Efter detta anrop syns den nya bilden på skärmen!
         */
        vga_flip();
    }
    
    /* ========== AVSLUTNING ========== */
    
    /*
     * Städa upp VGA
     * 
     * MYCKET VIKTIGT! Om vi inte gör detta kommer DOS
     * att starta i grafikläge och se trasigt ut.
     * 
     * vga_close():
     * 1. Återställer textläge (Mode 03h)
     * 2. Frigör dubbelbufferns minne
     */
    vga_close();
    
    /*
     * Skriv ut avslutningsmeddelande
     * 
     * cputs() skriver en sträng till skärmen.
     * \r\n = carriage return + newline (DOS-konvention)
     * 
     * Nu när vi är tillbaka i textläge fungerar
     * normal textutskrift igen.
     */
    cputs("Tack for att du spelade!\r\n");
    
    /*
     * Returnera 0 för lyckad körning
     * 
     * I C betyder return 0 från main() att programmet
     * avslutades utan fel. Andra värden indikerar fel.
     * DOS använder detta som "errorlevel".
     */
    return 0;
}
