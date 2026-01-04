/*
 * player.c - Spelarlogik och fysik
 * 
 * SYFTE:
 *   Denna modul hanterar spelarens tillstånd, rörelse och fysik.
 *   Den tar emot input (left, right, jump) och uppdaterar spelarens
 *   position baserat på fysik och kollisioner med världen.
 * 
 * SPELARPOSITIONEN:
 *   Spelarens position (x, y) är ÖVRE VÄNSTRA hörnet av spelarens
 *   kollisionsrektangel. Detta är standard i 2D-spel.
 *   
 *   Spelaren är 12 pixlar bred och 16 pixlar hög:
 *   
 *   (x,y) ───────────┐
 *      │             │ 16 px hög
 *      │   SPELARE   │
 *      │             │
 *      └─────────────┘
 *           12 px bred
 * 
 * FYSIKMODELL:
 *   Vi använder enkel "verlet-liknande" integration:
 *   1. Applicera gravitation på hastighet: vy += GRAVITY
 *   2. Applicera hastighet på position: y += vy
 *   3. Hantera kollisioner och justera position
 *   
 *   Detta är inte fysikaliskt korrekt men "känns" bra för plattformsspel.
 * 
 * BEROENDEN:
 *   - player.h: Player-strukturen och konstanter
 *   - level.h: Kollisionsdetektering mot plattformar
 *   - vga.h: Rita spelaren
 *   - types.h: SCREEN_W/H konstanter
 */

#include "player.h"
#include "level.h"
#include "vga.h"
#include "types.h"
#include "sound.h"

/*
 * GRAVITY - Hur mycket hastigheten ökar varje frame
 * 
 * Värde 1 betyder: varje frame ökar vy med 1 pixel/frame.
 * Efter 10 frames faller spelaren 10 pixlar/frame.
 * 
 * ENHETER:
 *   Hastighet mäts i pixlar per frame.
 *   VGA uppdaterar ~70 FPS, så 1 pixel/frame ≈ 70 pixlar/sekund.
 */
#define GRAVITY 1

/*
 * JUMP_FORCE - Initial hastighet uppåt vid hopp
 * 
 * NEGATIVT värde eftersom Y-axeln är inverterad:
 *   - Y=0 är TOPPEN av skärmen
 *   - Y=199 är BOTTEN av skärmen
 *   - Negativ vy = rörelse UPPÅT
 * 
 * Med JUMP_FORCE=-8 och GRAVITY=1:
 *   Frame 0: vy = -8 (rör sig upp 8 px)
 *   Frame 1: vy = -7 (rör sig upp 7 px)
 *   ...
 *   Frame 8: vy = 0  (toppen av hoppet)
 *   Frame 9: vy = 1  (börjar falla)
 *   ...
 * 
 * Total hopphöjd ≈ 8+7+6+5+4+3+2+1 = 36 pixlar
 */
#define JUMP_FORCE -20

/*
 * MOVE_SPEED - Horisontell hastighet i pixlar per frame
 * 
 * 3 pixlar/frame × 70 FPS ≈ 210 pixlar/sekund
 * Skärmen är 320 pixlar bred, så det tar ~1.5 sekunder
 * att gå från ena sidan till den andra.
 */
#define MOVE_SPEED 3

/*
 * player_init - Initierar spelarens tillstånd
 * 
 * PARAMETRAR:
 *   p - Pekare till Player-strukturen som ska initieras
 *   x - Startposition X (0-319)
 *   y - Startposition Y (0-199)
 * 
 * FUNGERAR SÅ HÄR:
 *   Sätter spelarens position och nollställer fysikvariabler.
 *   Anropas vid spelstart och när spelaren "dör" (faller av skärmen).
 */
void player_init(Player *p, int x, int y) {
    p->x = x;           /* Horisontell position */
    p->y = y;           /* Vertikal position */
    p->vy = 0;          /* Vertikal hastighet (stillastående) */
    p->on_ground = 0;   /* Börjar i luften (faller till marken) */
}

/*
 * player_update - Uppdaterar spelarens position och fysik
 * 
 * PARAMETRAR:
 *   p     - Pekare till Player-strukturen
 *   left  - 1 om vänsterpil är nedtryckt, annars 0
 *   right - 1 om högerpil är nedtryckt, annars 0
 *   jump  - 1 om mellanslag trycktes denna frame, annars 0
 * 
 * FUNGERAR SÅ HÄR:
 *   1. Beräkna ny X-position baserat på input
 *   2. Kontrollera horisontell kollision
 *   3. Hantera hopp (om på marken och jump-input)
 *   4. Applicera gravitation
 *   5. Beräkna ny Y-position
 *   6. Kontrollera vertikal kollision (landa/slå i huvudet)
 *   7. Respawna om spelaren faller ut
 * 
 * VARFÖR SEPARATA X OCH Y STEG?
 *   Genom att hantera X och Y separat undviker vi problem
 *   där spelaren fastnar i hörn. Detta kallas "axis separation"
 *   och är standard i plattformsspel.
 */
void player_update(Player *p, int left, int right, int jump) {
    int new_x, new_y;   /* Kandidatposition efter rörelse */
    int collision;      /* Index för träffad plattform, eller -1 */
    
    /* ========== HORISONTELL RÖRELSE ========== */
    
    /*
     * Börja med nuvarande position
     * Vi beräknar en ny position och kontrollerar sedan
     * om den är giltig innan vi applicerar den.
     */
    new_x = p->x;
    
    /*
     * Applicera input
     * 
     * Om båda left och right är nedtryckta tar de ut varandra
     * (ny_x - MOVE_SPEED + MOVE_SPEED = new_x)
     */
    if (left) new_x -= MOVE_SPEED;
    if (right) new_x += MOVE_SPEED;
    
    /*
     * Skärmgränser (clipping)
     * 
     * Spelaren kan inte gå utanför skärmens kanter.
     * SCREEN_W - PLAYER_W = 320 - 12 = 308 är max X
     * (så att spelarens högra kant inte går utanför skärmen)
     */
    if (new_x < 0) new_x = 0;
    if (new_x > SCREEN_W - PLAYER_W) new_x = SCREEN_W - PLAYER_W;
    
    /*
     * Horisontell kollisionsdetektering
     * 
     * Kontrollera om den nya positionen kolliderar med en plattform.
     * level_check_collision() returnerar plattformsindex (>=0) om
     * det finns kollision, eller -1 om det är fritt.
     * 
     * Vi kollar kollision på nya X men GAMLA Y - detta är viktigt
     * för axis separation.
     */
    collision = level_check_collision(new_x, p->y, PLAYER_W, PLAYER_H);
    if (collision < 0) {
        /* Ingen kollision - applicera rörelsen */
        p->x = new_x;
    }
    /* Om kollision: behåll gamla p->x (rör dig inte in i plattformen) */
    
    /* ========== HOPP ========== */
    
    /*
     * Hopp-logik
     * 
     * Spelaren kan bara hoppa om:
     * 1. Jump-input är aktiv (mellanslag trycktes denna frame)
     * 2. Spelaren står på marken (on_ground == 1)
     * 
     * Detta förhindrar "lufthopp" (hoppa i luften).
     */
    if (jump && p->on_ground) {
        p->vy = JUMP_FORCE;
        p->on_ground = 0;
        sound_play(SFX_JUMP);
    }
    
    /* ========== GRAVITATION ========== */
    
    /*
     * Applicera gravitation på hastighet
     * 
     * Varje frame ökar vy med GRAVITY (1).
     * Om vy är negativ (rör sig uppåt) kommer den att
     * gradvis närma sig 0 och sedan bli positiv (börja falla).
     */
    p->vy += GRAVITY;
    
    /*
     * Terminal velocity (maximal fallhastighet)
     * 
     * Utan detta skulle spelaren accelerera för evigt.
     * Max 10 pixlar/frame förhindrar "för snabbt" fall
     * som kan göra att spelaren "glider igenom" plattformar.
     */
    if (p->vy > 10) p->vy = 10;
    
    /* ========== VERTIKAL RÖRELSE ========== */
    
    /*
     * Beräkna ny Y-position
     * 
     * Adderar hastigheten till positionen.
     * Positiv vy = rörelse nedåt (faller)
     * Negativ vy = rörelse uppåt (hoppar)
     */
    new_y = p->y + p->vy;
    
    /*
     * Anta att spelaren INTE är på marken
     * 
     * Vi nollställer on_ground och sätter den till 1
     * endast om vi detekterar kollision underifrån.
     */
    p->on_ground = 0;
    
    /*
     * Vertikal kollisionsdetektering
     * 
     * Kontrollera om ny position kolliderar med plattform.
     * Vi använder gamla X men nya Y.
     */
    collision = level_check_collision(p->x, new_y, PLAYER_W, PLAYER_H);
    
    if (collision >= 0 && p->vy > 0) {
        /*
         * LANDNING: Kollision medan vi faller nedåt
         * 
         * Spelaren träffade en plattform uppifrån.
         * Vi justerar Y så att spelarens fötter vilar exakt
         * på plattformens ovansida.
         * 
         * level_get_platform_top() returnerar plattformens Y-koordinat.
         * Vi subtraherar PLAYER_H så att spelarens BOTTEN
         * (inte toppen) vilar på plattformen.
         */
        p->y = level_get_platform_top(collision) - PLAYER_H;
        p->vy = 0;          /* Stoppa vertikal rörelse */
        p->on_ground = 1;   /* Nu står vi på marken */
        
    } else if (collision >= 0 && p->vy < 0) {
        /*
         * HUVUDKROCK: Kollision medan vi hoppar uppåt
         * 
         * Spelaren slog huvudet i undersidan av en plattform.
         * Vi stoppar uppåtrörelsen men applicerar inte
         * den nya positionen (så spelaren "studsar" nedåt).
         */
        p->vy = 0;  /* Stoppa uppåtrörelse, börja falla */
        
    } else {
        /*
         * INGEN KOLLISION: Fri rörelse
         * 
         * Spelaren är i luften - applicera den nya positionen.
         */
        p->y = new_y;
    }
    
    /* ========== RESPAWN ========== */
    
    /*
     * Kontrollera om spelaren föll av skärmen
     * 
     * Om Y > SCREEN_H (200) har spelaren fallit under skärmen.
     * Vi "respawnar" spelaren till startpositionen.
     * 
     * I ett riktigt spel skulle detta kanske kosta ett liv.
     */
    if (p->y > SCREEN_H) {
        player_init(p, 50, 150);  /* Återställ till startposition */
    }
}

/*
 * player_draw - Ritar spelaren på skärmen
 * 
 * PARAMETRAR:
 *   p - Pekare till Player-strukturen med position
 * 
 * FUNGERAR SÅ HÄR:
 *   Ritar en enkel "gubbe" som två rektanglar:
 *   - En större gul rektangel (kroppen)
 *   - En mindre röd rektangel (huvudet/ansiktet)
 * 
 * I ETT RIKTIGT SPEL:
 *   Här skulle man rita en sprite (bitmap) istället för rektanglar.
 *   Man skulle också animera baserat på rörelsestillstånd
 *   (gå, hoppa, stå stilla, etc.)
 */
void player_draw(Player *p) {
    /*
     * Rita kroppen (gul rektangel)
     * 
     * Färg 14 = Gul i standard VGA-paletten.
     * Storleken är PLAYER_W × PLAYER_H (12 × 16 pixlar).
     */
    vga_draw_rect(p->x, p->y, PLAYER_W, PLAYER_H, 14);
    
    /*
     * Rita huvudet/ansiktet (röd rektangel)
     * 
     * Färg 4 = Mörkröd i standard VGA-paletten.
     * Offset med 2 pixlar från hörnet för att centrera.
     * Storleken är 8 × 6 pixlar.
     */
    vga_draw_rect(p->x + 2, p->y + 2, 8, 6, 4);
}
