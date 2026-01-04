# Arkitektur - Retro DOS Platformer

## Översikt

Detta dokument beskriver projektets arkitektur och hur de olika modulerna
samarbetar för att skapa ett fungerande DOS-spel.

```
┌─────────────────────────────────────────────────────────────────┐
│                         main.c                                   │
│                    (Spelloop & Koordinering)                     │
├─────────────────────────────────────────────────────────────────┤
│                              │                                   │
│         ┌────────────────────┼────────────────────┐             │
│         │                    │                    │             │
│         ▼                    ▼                    ▼             │
│    ┌─────────┐         ┌──────────┐         ┌─────────┐        │
│    │ input.c │         │ player.c │         │ level.c │        │
│    │         │         │          │         │         │        │
│    │Tangent- │────────▶│ Spelar-  │◀───────▶│ Nivå-   │        │
│    │ bord    │  input  │  logik   │kollision│  data   │        │
│    └─────────┘         └──────────┘         └─────────┘        │
│                              │                    │             │
│                              │ rita               │ rita        │
│                              ▼                    ▼             │
│                        ┌──────────────────────────────┐        │
│                        │           vga.c              │        │
│                        │     (VGA Mode 13h Grafik)    │        │
│                        └──────────────────────────────┘        │
│                                      │                          │
│                                      ▼                          │
│                        ┌──────────────────────────────┐        │
│                        │      VGA HÅRDVARA            │        │
│                        │    Framebuffer 0xA0000       │        │
│                        └──────────────────────────────┘        │
└─────────────────────────────────────────────────────────────────┘
```

## Mappstruktur

```
retro-game/
├── src/                    # Källkod (implementation)
│   ├── main.c              # Entry point och spelloop
│   ├── vga.c               # VGA-grafikimplementation
│   ├── input.c             # Tangentbordshantering
│   ├── player.c            # Spelarlogik och fysik
│   ├── level.c             # Nivådata och kollision
│   ├── menu.c              # Startmeny och pausmeny
│   ├── game.c              # Spelinstans och save/load
│   ├── sound.c             # Ljudeffekter (SB + PC Speaker)
│   └── mixer.c             # Software audio mixer
│
├── include/                # Header-filer (gränssnitt)
│   ├── types.h             # Gemensamma typer och konstanter
│   ├── vga.h               # VGA-funktionsdeklarationer
│   ├── input.h             # Input-strukturer och funktioner
│   ├── player.h            # Player-struktur och funktioner
│   ├── level.h             # Nivåfunktioner
│   ├── menu.h              # Meny-deklarationer
│   ├── game.h              # GameState och save/load
│   ├── sound.h             # Ljud-API
│   └── mixer.h             # Mixer-API
│
├── SOUNDS/                 # Ljudfiler
│   └── MAINMENU.WAV        # Menymusik (8-bit PCM)
│
├── docs/                   # Dokumentation
│   ├── ARCHITECTURE.md     # Detta dokument
│   ├── BUILDING.md         # Kompileringsinstruktioner
│   └── LEARNING.md         # Lärresurser
│
└── .windsurf/              # IDE-konfiguration
    ├── rules/              # AI-regler för projektet
    └── workflows/          # Interaktiva kommandon
```

## Moduler

### main.c - Koordinator

**Ansvar:** Spelloop och initiering av alla moduler.

**Spelloop (körs ~70 gånger/sekund):**
```
while (!quit) {
    1. INPUT:  Läs tangentbord
    2. UPDATE: Uppdatera spelarfysik
    3. RENDER: Rita allt
    4. PRESENT: Visa på skärm
}
```

### vga.c - Grafik

**Ansvar:** ALL kommunikation med VGA-hårdvaran.

**Funktioner:**
| Funktion | Beskrivning |
|----------|-------------|
| `vga_init()` | Sätter Mode 13h, allokerar buffert |
| `vga_close()` | Återställer textläge, frigör minne |
| `vga_clear(color)` | Fyller skärmen med en färg |
| `vga_draw_rect()` | Ritar en rektangel |
| `vga_vsync()` | Väntar på vertical retrace |
| `vga_flip()` | Kopierar buffert till VGA-minne |

**Tekniska detaljer:**
- Mode 13h: 320×200, 256 färger
- Framebuffer: 0xA0000 (64000 bytes)
- Dubbelbuffring för flimmerfri rendering

### input.c - Tangentbord

**Ansvar:** Läsa tangentbordsinput.

**Hanterar:**
- Piltangenter (extended keys med scan codes)
- Mellanslag (hopp)
- ESC (avsluta)

**InputState struktur:**
```c
typedef struct {
    int left, right;  /* Piltangenter */
    int up, down;     /* Menynavigering */
    int jump;         /* Mellanslag */
    int enter;        /* Enter (menyval) */
    int pause;        /* P-tangent */
    int quit;         /* ESC */
} InputState;
```

### player.c - Spelarlogik

**Ansvar:** Spelarens fysik, rörelse och rendering.

**Fysikmodell:**
- Gravitation: `vy += GRAVITY` varje frame
- Hopp: `vy = JUMP_FORCE` (negativ = uppåt)
- Terminal velocity: max fallhastighet 10 px/frame
- Axis separation: X och Y hanteras separat

**Player struktur:**
```c
typedef struct {
    int x, y;       /* Position (övre vänstra hörnet) */
    int vy;         /* Vertikal hastighet */
    int on_ground;  /* 1 om på mark, 0 om i luften */
} Player;
```

### level.c - Nivådata

**Ansvar:** Plattformsdata och kollisionsdetektering.

**AABB Kollision:**
Två rektanglar kolliderar om de överlappar på BÅDE X och Y:
```
A.left < B.right  OCH  A.right > B.left
OCH
A.top < B.bottom  OCH  A.bottom > B.top
```

## Dataflöde

```
┌──────────┐    InputState    ┌──────────┐
│ input.c  │─────────────────▶│  main.c  │
└──────────┘                  └────┬─────┘
                                   │
                    left, right, jump
                                   │
                                   ▼
                            ┌──────────┐
                            │ player.c │
                            └────┬─────┘
                                 │
                    x, y, w, h (kollisionsfråga)
                                 │
                                 ▼
                            ┌──────────┐
                            │ level.c  │──▶ plattformsindex / -1
                            └──────────┘
```

## Beroenden

```
main.c ──────▶ vga.h, input.h, player.h, level.h, menu.h, game.h
player.c ────▶ player.h, level.h, vga.h, types.h
level.c ─────▶ level.h, vga.h
vga.c ───────▶ vga.h, types.h
input.c ─────▶ input.h
menu.c ──────▶ menu.h, vga.h, input.h
game.c ──────▶ game.h, player.h
```

**Regel:** Inga cirkulära beroenden! vga.c känner INTE till player.c.

### menu.c - Menysystem

**Ansvar:** Startmeny och pausmeny med bitmap-font.

**Funktioner:**
- `menu_show(has_save)` - Huvudmeny (NEW GAME, CONTINUE, QUIT)
- `pause_show()` - Pausmeny (RESUME, SAVE, MENU)

**Helper-funktioner:**
- `clear_input()` - Rensar tangentbordsbuffern
- `wait_key_release()` - Debounce för menynavigering

### game.c - Spelinstans

**Ansvar:** Spara och ladda speldata (DOS-style binärfil).

**GameState struktur:**
```c
typedef struct {
    int player_x, player_y;   /* Spelarposition */
    int player_vy;            /* Hastighet */
    int score, level;         /* Statistik */
    unsigned int magic;       /* Verifieringsnummer */
} GameState;
```

**Funktioner:**
- `game_new()` - Skapar ny spelinstans
- `game_save()` - Sparar till GAME.SAV
- `game_load()` - Laddar från GAME.SAV
- `game_exists()` - Kontrollerar om sparfil finns

### sound.c - Ljudeffekter

**Ansvar:** Högnivå-API för ljud, PC Speaker fallback.

**Funktioner:**
| Funktion | Beskrivning |
|----------|-------------|
| `sound_init()` | Detekterar Sound Blaster |
| `sound_close()` | Frigör resurser |
| `sound_play(effect)` | Spelar ljudeffekt |
| `music_play(file, loop)` | Startar musik-streaming |
| `music_stop()` | Stoppar musik |
| `music_update()` | Uppdaterar mixer (anropa varje frame!) |

**Ljudeffekter:**
- `SFX_JUMP` - Hopp
- `SFX_MENU_MOVE` - Menynavigering
- `SFX_MENU_SELECT` - Menyval
- `SFX_LAND` - Landning

### mixer.c - Audio Mixer

**Ansvar:** Software mixing av musik + SFX via Sound Blaster.

**Arkitektur:**
```
┌─────────────────────────────────────────────────┐
│                  mixer.c                         │
├─────────────────────────────────────────────────┤
│  ┌──────────┐    ┌──────────┐                   │
│  │ Buffer A │◀──▶│ Buffer B │  Double-buffer    │
│  └──────────┘    └──────────┘                   │
│       │                                          │
│       ▼                                          │
│  ┌──────────────────────────────────┐           │
│  │         mixer_fill_buffer()      │           │
│  │   Musik (WAV) + SFX → Mixad PCM  │           │
│  └──────────────────────────────────┘           │
│       │                                          │
│       ▼                                          │
│  ┌──────────────────────────────────┐           │
│  │         DMA Kanal 1              │           │
│  │      Auto-init mode              │           │
│  └──────────────────────────────────┘           │
│       │                                          │
│       ▼                                          │
│  ┌──────────────────────────────────┐           │
│  │      Sound Blaster DSP           │           │
│  │         Port 0x220               │           │
│  └──────────────────────────────────┘           │
└─────────────────────────────────────────────────┘
```

**Tekniska detaljer:**
- Buffer: 2048 bytes (~185ms latens)
- Sample rate: 11025 Hz
- Format: 8-bit unsigned PCM
- Max 4 samtidiga SFX-kanaler
- IRQ 7 för buffert-byte
