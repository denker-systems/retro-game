/*
 * level.c - Nivådata och kollisioner
 * 
 * SYFTE:
 *   Denna modul hanterar spelvärlden - plattformarna som spelaren
 *   kan stå på och kollidera med. Den tillhandahåller:
 *   - Nivådata (plattformarnas positioner och storlekar)
 *   - Kollisionsdetektering (AABB - Axis-Aligned Bounding Box)
 *   - Ritning av nivån
 * 
 * DATASTRUKTUR:
 *   Plattformar lagras som en 2D-array av integers.
 *   Varje plattform har 4 värden: [x, y, bredd, höjd]
 *   
 *   I ett större spel skulle man använda:
 *   - En struct för varje plattform (mer läsbart)
 *   - Ladda nivådata från en fil
 *   - Stöd för olika typer av plattformar (rörliga, farliga, etc.)
 * 
 * KOLLISIONSDETEKTERING - AABB:
 *   AABB = Axis-Aligned Bounding Box
 *   "Axis-aligned" betyder att rektanglarna inte är roterade.
 *   
 *   Två AABB:er kolliderar om och endast om de överlappar
 *   på BÅDE X-axeln OCH Y-axeln samtidigt.
 *   
 *   Matematiskt: rektanglarna överlappar INTE om:
 *   - A är helt till vänster om B (A.right < B.left)
 *   - A är helt till höger om B (A.left > B.right)
 *   - A är helt ovanför B (A.bottom < B.top)
 *   - A är helt under B (A.top > B.bottom)
 *   
 *   Om INGET av dessa är sant -> kollision!
 * 
 * BEROENDEN:
 *   - level.h: Funktionsdeklarationer
 *   - vga.h: vga_draw_rect() för att rita plattformar
 */

#include "level.h"
#include "vga.h"

/*
 * platforms - Array med alla plattformar i nivån
 * 
 * FORMAT: {x, y, bredd, höjd}
 *   x     = Vänster kant (0-319)
 *   y     = Övre kant (0-199)
 *   bredd = Bredd i pixlar
 *   höjd  = Höjd i pixlar
 * 
 * KOORDINATSYSTEM:
 *   (0,0) är övre vänstra hörnet av skärmen
 *   X ökar åt höger
 *   Y ökar NEDÅT (inverterat jämfört med matematik)
 * 
 * LAYOUT:
 *                     ┌────────────┐
 *                     │ Topp (100,50)
 *                     └────────────┘
 *   
 *                            ┌──────┐
 *                            │Plattform 3 (240,80)
 *                            └──────┘
 *   
 *              ┌──────┐
 *              │Plattform 2 (150,110)
 *              └──────┘
 *   
 *   ┌──────┐
 *   │Plattform 1 (50,140)
 *   └──────┘
 *   
 *   ════════════════════════════════════
 *            Marken (0,180)
 * 
 * static = Variabeln är privat till denna fil
 * [][4]  = 2D-array med 4 kolumner per rad
 */
static int platforms[][4] = {
    {0, 180, 320, 20},   /* Index 0: Marken (hela skärmbredden) */
    {50, 140, 60, 10},   /* Index 1: Första plattformen */
    {150, 110, 60, 10},  /* Index 2: Andra plattformen */
    {240, 80, 60, 10},   /* Index 3: Tredje plattformen */
    {100, 50, 80, 10}    /* Index 4: Toppplattformen */
};

/*
 * NUM_PLATFORMS - Antal plattformar i arrayen
 * 
 * Vi definierar detta manuellt för enkelhet.
 * Alternativt: sizeof(platforms) / sizeof(platforms[0])
 */
#define NUM_PLATFORMS 5

/*
 * level_init - Initierar nivån
 * 
 * ANROPAS: En gång vid spelstart
 * 
 * NUVARANDE IMPLEMENTATION:
 *   Gör ingenting - nivådata är hårdkodad.
 * 
 * FRAMTIDA FÖRBÄTTRINGAR:
 *   - Ladda nivådata från en .MAP-fil
 *   - Generera procedurella nivåer
 *   - Initiera rörliga plattformar
 */
void level_init(void) {
    /* Framtida: ladda nivå från fil */
}

/*
 * level_draw - Ritar alla plattformar
 * 
 * ANROPAS: Varje frame, före player_draw()
 * 
 * FUNGERAR SÅ HÄR:
 *   Loopar genom alla plattformar och ritar dem.
 *   Marken (index 0) ritas i brun, övriga i grön.
 * 
 * RITORDNING:
 *   I 2D-spel ritas saker i ordning bakifrån och framåt.
 *   Saker som ritas senare hamnar "ovanpå" tidigare saker.
 *   Därför ritar vi: bakgrund -> nivå -> spelare -> UI
 */
void level_draw(void) {
    int i;  /* Loop-räknare - deklareras här (C89-krav) */
    
    /* Loopa genom alla plattformar */
    for (i = 0; i < NUM_PLATFORMS; i++) {
        /*
         * Rita plattformen
         * 
         * Ternary operator: (villkor) ? sant-värde : falskt-värde
         * 
         * Om i == 0 (marken): använd färg 6 (brun)
         * Annars: använd färg 2 (mörkgrön)
         */
        vga_draw_rect(
            platforms[i][0],  /* x */
            platforms[i][1],  /* y */
            platforms[i][2],  /* bredd */
            platforms[i][3],  /* höjd */
            i == 0 ? 6 : 2    /* färg: brun för mark, grön för plattformar */
        );
    }
}

/*
 * level_check_collision - Kollar om en rektangel kolliderar med plattformar
 * 
 * PARAMETRAR:
 *   x, y - Övre vänstra hörnet av rektangeln att testa
 *   w, h - Bredd och höjd av rektangeln
 * 
 * RETURNERAR:
 *   >= 0: Index för den första plattformen som kolliderar
 *   -1:   Ingen kollision
 * 
 * ALGORITM - AABB Kollisionsdetektion:
 *   Två rektanglar A och B överlappar om och endast om:
 *   
 *   A.left < B.right  OCH  A.right > B.left
 *   OCH
 *   A.top < B.bottom  OCH  A.bottom > B.top
 *   
 *   I kod:
 *   x < px + pw      (A:s vänsterkant är till vänster om B:s högerkant)
 *   x + w > px       (A:s högerkant är till höger om B:s vänsterkant)
 *   y < py + ph      (A:s överkant är ovanför B:s underkant)
 *   y + h > py       (A:s underkant är under B:s överkant)
 * 
 * VISUALISERING:
 *   
 *   Kollision:           Ingen kollision:
 *   
 *   ┌────┐               ┌────┐
 *   │ A ┌┼───┐           │ A  │     ┌────┐
 *   └───┼┘ B │           └────┘     │ B  │
 *       └────┘                      └────┘
 */
int level_check_collision(int x, int y, int w, int h) {
    int i;  /* Loop-räknare */
    
    /* Testa mot varje plattform */
    for (i = 0; i < NUM_PLATFORMS; i++) {
        /* Hämta plattformens data för läsbarhet */
        int px = platforms[i][0];  /* Plattformens X */
        int py = platforms[i][1];  /* Plattformens Y */
        int pw = platforms[i][2];  /* Plattformens bredd */
        int ph = platforms[i][3];  /* Plattformens höjd */
        
        /*
         * AABB kollisionstest
         * 
         * Alla fyra villkor måste vara sanna för kollision.
         * Vi kombinerar dem med && (logiskt OCH).
         */
        if (x < px + pw &&    /* Spelarens vänster < plattformens höger */
            x + w > px &&     /* Spelarens höger > plattformens vänster */
            y < py + ph &&    /* Spelarens topp < plattformens botten */
            y + h > py) {     /* Spelarens botten > plattformens topp */
            
            /*
             * Kollision detekterad!
             * Returnera plattformens index så att anroparen
             * kan veta VILKEN plattform som träffades.
             */
            return i;
        }
    }
    
    /*
     * Ingen kollision hittades
     * Returnera -1 som "ingen träff"-markör.
     */
    return -1;
}

/*
 * level_get_platform_top - Hämtar Y-koordinaten för en plattforms ovansida
 * 
 * PARAMETRAR:
 *   index - Plattformens index (0 till NUM_PLATFORMS-1)
 * 
 * RETURNERAR:
 *   Plattformens Y-koordinat (dess "topp")
 *   Om ogiltigt index: returnerar SCREEN_H (skärmens botten)
 * 
 * ANVÄNDNING:
 *   När spelaren landar på en plattform behöver player.c veta
 *   exakt var plattformens ovansida är för att placera spelaren.
 *   
 *   player.y = level_get_platform_top(index) - PLAYER_H;
 *   
 *   Detta placerar spelarens FÖTTER på plattformens ovansida.
 */
int level_get_platform_top(int index) {
    /* Validera index för att undvika array-out-of-bounds */
    if (index >= 0 && index < NUM_PLATFORMS) {
        /*
         * Returnera plattformens Y-koordinat
         * 
         * platforms[index][1] är Y-värdet (andra kolumnen)
         * Kom ihåg: Y är TOPPEN av plattformen i vårt koordinatsystem
         */
        return platforms[index][1];
    }
    
    /*
     * Ogiltigt index - returnera skärmens botten som fallback
     * Detta borde aldrig hända om koden är korrekt.
     */
    return SCREEN_H;
}
