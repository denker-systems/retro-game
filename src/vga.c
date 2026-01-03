/*
 * vga.c - VGA Mode 13h grafikimplementation
 * 
 * SYFTE:
 *   Denna modul hanterar all kommunikation med VGA-grafikkortet.
 *   Den abstraherar bort hårdvarudetaljerna så att resten av spelet
 *   kan rita grafik utan att veta hur VGA fungerar internt.
 * 
 * HÅRDVARA - VGA (Video Graphics Array):
 *   VGA introducerades av IBM 1987 och blev snabbt standarden för PC-grafik.
 *   Mode 13h är det enklaste grafikläget att programmera:
 *   
 *   - Upplösning: 320 x 200 pixlar
 *   - Färger: 256 samtidiga färger (från en palett på 262,144)
 *   - Minne: 64,000 bytes (320 * 200 = 64000)
 *   - Adress: Videominnet börjar på fysisk adress 0xA0000
 *   
 *   Varje pixel representeras av EN byte (8 bitar).
 *   Bytens värde (0-255) är ett index i färgpaletten.
 * 
 * VARFÖR MODE 13h?
 *   Till skillnad från andra VGA-lägen har Mode 13h en LINJÄR framebuffer.
 *   Det betyder att pixlarna ligger i ordning i minnet:
 *   - Pixel (0,0) är på adress 0xA0000 + 0
 *   - Pixel (1,0) är på adress 0xA0000 + 1
 *   - Pixel (0,1) är på adress 0xA0000 + 320
 *   
 *   Formel: adress = 0xA0000 + (y * 320) + x
 * 
 * DOUBLE BUFFERING:
 *   Om vi ritar direkt till VGA-minnet medan skärmen uppdateras
 *   får vi "tearing" (synliga artefakter). Lösningen är double buffering:
 *   1. Rita allt till en buffer i RAM
 *   2. Vänta på "vertical retrace" (när elektronkanonen är mellan frames)
 *   3. Kopiera hela buffern till VGA-minnet på en gång
 * 
 * BEROENDEN:
 *   - dos.h: int86() för BIOS-anrop, inp() för port-läsning
 *   - malloc.h: _fmalloc()/_ffree() för far heap-allokering
 *   - types.h: byte typedef
 */

#include <dos.h>      /* int86() för BIOS-interrupts, inp() för I/O-portar */
#include <string.h>   /* Behövs för _fmemset/_fmemcpy i vissa kompilatorer */
#include <malloc.h>   /* _fmalloc() och _ffree() för far heap */
#include "vga.h"

/*
 * VGA - Pekare till VGA-videominnet
 * 
 * ADRESS: 0xA0000 (fysisk adress i PC:ns minnesrymd)
 * 
 * VARFÖR 0xA0000000L?
 *   I DOS real mode används "segment:offset"-adressering.
 *   En far-pekare innehåller både segment och offset.
 *   
 *   0xA0000000L bryts ner så här:
 *   - Segment: 0xA000 (de övre 16 bitarna)
 *   - Offset:  0x0000 (de undre 16 bitarna)
 *   
 *   Fysisk adress = (segment * 16) + offset
 *                 = (0xA000 * 16) + 0
 *                 = 0xA0000
 * 
 * VARFÖR "far"?
 *   I "small" memory model (som vi använder) är vanliga pekare "near"
 *   och kan bara adressera inom programmets datasegment (64KB).
 *   VGA-minnet ligger UTANFÖR vårt segment, så vi måste använda
 *   en "far" pekare som innehåller både segment och offset.
 */
static byte far *VGA = (byte far *)0xA0000000L;

/*
 * buffer - Vår dubbelbuffer (back buffer)
 * 
 * Vi ritar ALDRIG direkt till VGA-minnet. Istället:
 * 1. Ritar vi till denna buffer (i vanligt RAM)
 * 2. När bilden är klar kopierar vi allt till VGA på en gång
 * 
 * Detta förhindrar flimmer och tearing.
 */
static byte far *buffer = NULL;

/*
 * vga_init - Initierar VGA Mode 13h och allokerar dubbelbuffer
 * 
 * ANROPAS: En gång vid spelets start
 * 
 * FUNGERAR SÅ HÄR:
 *   1. Allokerar 64000 bytes för dubbelbuffern
 *   2. Anropar BIOS för att sätta grafikläge
 * 
 * BIOS INTERRUPT 10h - Videotjänster:
 *   INT 10h är BIOS:ens video-interrupt. Genom att sätta olika
 *   värden i CPU-registren kan vi be BIOS göra olika saker.
 *   
 *   AH = 0x00: "Set Video Mode" funktion
 *   AL = 0x13: Mode 13h (320x200, 256 färger)
 *   
 *   När int86() anropas händer detta:
 *   1. CPU:n sparar alla register på stacken
 *   2. CPU:n hoppar till BIOS-koden för INT 10h
 *   3. BIOS programmerar VGA-hårdvaran
 *   4. CPU:n återställer registren och återvänder
 */
void vga_init(void) {
    /*
     * union REGS - En union för att komma åt CPU-register
     * 
     * I x86-arkitekturen finns register som AX, BX, CX, DX.
     * Varje 16-bit register kan delas i två 8-bit halvor:
     *   AX = AH (high byte) + AL (low byte)
     * 
     * union REGS låter oss komma åt båda:
     *   regs.x.ax = hela 16-bit registret
     *   regs.h.ah = övre 8 bitar
     *   regs.h.al = undre 8 bitar
     */
    union REGS regs;
    
    /*
     * Allokera dubbelbuffer i "far heap"
     * 
     * _fmalloc() allokerar minne utanför vårt datasegment.
     * Vi behöver 64000 bytes (320 * 200 pixlar * 1 byte/pixel).
     * 
     * VIKTIGT: I ett riktigt spel bör vi kontrollera att
     * allokeringen lyckades (buffer != NULL).
     */
    buffer = (byte far *)_fmalloc(64000U);
    
    /*
     * Sätt videoläge via BIOS
     * 
     * AH = 0x00 betyder "Set Video Mode"
     * AL = 0x13 betyder Mode 13h
     * 
     * Andra vanliga lägen:
     *   0x03 = 80x25 textläge (standard DOS)
     *   0x12 = 640x480, 16 färger (VGA)
     *   0x13 = 320x200, 256 färger (det vi använder)
     */
    regs.h.ah = 0x00;  /* Funktion: Set Video Mode */
    regs.h.al = 0x13;  /* Läge: Mode 13h */
    int86(0x10, &regs, &regs);  /* Anropa BIOS video interrupt */
}

/*
 * vga_close - Återställer textläge och frigör minne
 * 
 * ANROPAS: När spelet avslutas
 * 
 * VIKTIGT:
 *   Om vi INTE återställer textläget kommer DOS-prompten
 *   att visas i grafikläge, vilket ser trasigt ut.
 *   Alltid återställ till Mode 03h (textläge) vid avslut!
 */
void vga_close(void) {
    union REGS regs;
    
    /*
     * Återställ textläge (Mode 03h)
     * 
     * Mode 03h är standard DOS-textläge:
     * - 80 kolumner x 25 rader
     * - 16 färger
     * - Texten visas normalt igen
     */
    regs.h.ah = 0x00;  /* Funktion: Set Video Mode */
    regs.h.al = 0x03;  /* Läge: 80x25 text */
    int86(0x10, &regs, &regs);
    
    /*
     * Frigör dubbelbuffern
     * 
     * _ffree() frigör minne allokerat med _fmalloc().
     * Vi sätter pekaren till NULL efteråt för att undvika
     * "dangling pointer" - en pekare till frigjort minne.
     */
    if (buffer) {
        _ffree(buffer);
        buffer = NULL;
    }
}

/*
 * vga_clear - Fyller hela skärmbuffern med en färg
 * 
 * PARAMETRAR:
 *   color - Färgindex 0-255 från VGA-paletten
 * 
 * VANLIGA FÄRGER (standard VGA-palett):
 *   0  = Svart          8  = Mörkgrå
 *   1  = Mörkblå        9  = Ljusblå
 *   2  = Mörkgrön       10 = Ljusgrön
 *   4  = Mörkröd        12 = Ljusröd
 *   6  = Brun           14 = Gul
 *   7  = Ljusgrå        15 = Vit
 * 
 * FUNGERAR SÅ HÄR:
 *   _fmemset() fyller ett minnesområde med ett byte-värde.
 *   Vi fyller alla 64000 bytes med färgvärdet.
 */
void vga_clear(byte color) {
    /*
     * _fmemset - Far memory set
     * 
     * Fungerar som memset() men för far-pekare.
     * Fyller 64000 bytes (hela skärmen) med färgen.
     */
    _fmemset(buffer, color, 64000U);
}

/*
 * vga_draw_rect - Ritar en fylld rektangel
 * 
 * PARAMETRAR:
 *   x     - Vänster kant (0-319), kan vara negativ för clipping
 *   y     - Övre kant (0-199), kan vara negativ för clipping
 *   w     - Bredd i pixlar
 *   h     - Höjd i pixlar
 *   color - Färgindex 0-255
 * 
 * FUNGERAR SÅ HÄR:
 *   Loopar genom varje rad och kolumn inom rektangeln.
 *   För varje pixel beräknas minnesadressen och färgen skrivs.
 * 
 * CLIPPING:
 *   Om rektangeln delvis är utanför skärmen "klipps" den.
 *   Pixlar utanför 0-319 (x) eller 0-199 (y) ritas inte.
 */
void vga_draw_rect(int x, int y, int w, int h, byte color) {
    int i, j;          /* Loop-räknare för kolumn och rad */
    byte far *dest;    /* Pekare till aktuell rad i buffern */
    
    /* Loopa genom varje rad i rektangeln */
    for (j = 0; j < h; j++) {
        /*
         * Clipping: Hoppa över rader utanför skärmen
         * 
         * Om y+j < 0 är raden ovanför skärmen
         * Om y+j >= 200 är raden under skärmen
         */
        if (y + j < 0 || y + j >= SCREEN_H) continue;
        
        /*
         * Beräkna adressen till radens början
         * 
         * Formel: buffer + (rad * 320) + kolumn
         * 
         * Exempel: För att rita på (50, 30):
         *   offset = 30 * 320 + 50 = 9650
         *   adress = buffer + 9650
         */
        dest = buffer + (y + j) * SCREEN_W + x;
        
        /* Loopa genom varje pixel i raden */
        for (i = 0; i < w; i++) {
            /*
             * Clipping: Hoppa över pixlar utanför skärmen
             * 
             * Vi kontrollerar varje pixel individuellt för att
             * hantera rektanglar som delvis är utanför skärmen.
             */
            if (x + i >= 0 && x + i < SCREEN_W) {
                dest[i] = color;  /* Skriv färgen till buffern */
            }
        }
    }
}

/*
 * vga_flip - Kopierar dubbelbuffern till VGA-minnet
 * 
 * ANROPAS: En gång per frame, efter all ritning är klar
 * 
 * FUNGERAR SÅ HÄR:
 *   Kopierar alla 64000 bytes från vår buffer till VGA-minnet.
 *   Detta uppdaterar hela skärmen på en gång.
 * 
 * VARFÖR INTE KOPIERA VARJE PIXEL SEPARAT?
 *   _fmemcpy() är optimerad och kopierar flera bytes åt gången.
 *   På en 486 kan detta göra stor skillnad i prestanda.
 */
void vga_flip(void) {
    /*
     * _fmemcpy - Far memory copy
     * 
     * Kopierar 64000 bytes från buffer till VGA-minnet.
     * Detta gör att hela den nya bilden visas på skärmen.
     */
    _fmemcpy(VGA, buffer, 64000U);
}

/*
 * vga_vsync - Väntar på vertical retrace (vsync)
 * 
 * ANROPAS: Innan vga_flip() för att undvika tearing
 * 
 * HÅRDVARA - VGA Status Register:
 *   Port 0x3DA är VGA:s "Input Status Register 1"
 *   Bit 3 (värde 8) indikerar vertical retrace:
 *   - Bit 3 = 0: Skärmen ritas just nu
 *   - Bit 3 = 1: Vertical retrace pågår (säkert att uppdatera)
 * 
 * VARFÖR VÄNTA?
 *   En CRT-skärm ritar bilden rad för rad, uppifrån och ner.
 *   "Vertical retrace" är tiden mellan sista raden och första raden
 *   på nästa frame. Under denna tid ritas ingenting på skärmen.
 *   
 *   Om vi kopierar till VGA-minnet MEDAN skärmen ritas,
 *   kan övre halvan visa den gamla bilden och undre halvan
 *   den nya - detta kallas "tearing".
 *   
 *   Genom att vänta på vertical retrace säkerställer vi att
 *   hela skärmen uppdateras mellan frames.
 * 
 * TIMING:
 *   VGA i Mode 13h uppdaterar skärmen ~70 gånger per sekund.
 *   Vertical retrace tar ca 1.3ms av varje ~14.3ms frame.
 */
void vga_vsync(void) {
    /*
     * Steg 1: Vänta tills vi INTE är i retrace
     * 
     * Om vi redan är i retrace vill vi vänta tills nästa.
     * inp(0x3DA) läser statusregistret.
     * "& 8" maskerar ut bit 3 (retrace-biten).
     * 
     * while (inp(0x3DA) & 8) - loopa medan bit 3 är 1
     */
    while (inp(0x3DA) & 8);
    
    /*
     * Steg 2: Vänta tills retrace BÖRJAR
     * 
     * Nu väntar vi på att retrace ska starta.
     * "!(... & 8)" är sant när bit 3 är 0.
     * 
     * while (!(inp(0x3DA) & 8)) - loopa medan bit 3 är 0
     * 
     * När loopen avslutas har vertical retrace just börjat
     * och det är säkert att kopiera till VGA-minnet.
     */
    while (!(inp(0x3DA) & 8));
}
