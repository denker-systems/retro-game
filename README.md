# Retro DOS Platformer

Ett minimalt Mario-liknande plattformsspel för MS-DOS, skrivet i C.

![Spelet i action](docs/screenshot.png)

## Dokumentation

| Dokument | Beskrivning |
|----------|-------------|
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Projektstruktur och moduldesign |
| [BUILDING.md](docs/BUILDING.md) | Kompilering och körning |
| [LEARNING.md](docs/LEARNING.md) | Lärresurser och övningar |
| [VGA_REFERENCE.md](docs/VGA_REFERENCE.md) | VGA Mode 13h teknisk referens |

## Projektstruktur

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
│   ├── menu.h              # Meny-funktioner
│   ├── game.h              # GameState och save/load
│   ├── sound.h             # Ljud-API
│   └── mixer.h             # Mixer-API
│
├── SOUNDS/                 # Ljudfiler (WAV)
│   └── MAINMENU.WAV        # Menymusik
│
└── .windsurf/              # IDE-konfiguration
    ├── rules/              # AI-regler för projektet
    └── workflows/          # Interaktiva kommandon
```

## Krav

- **Open Watcom 2.0** - https://open-watcom.github.io/
- **DOSBox-X** - https://dosbox-x.com/

## Installation (Windows)

1. Ladda ner Open Watcom från https://github.com/open-watcom/open-watcom-v2/releases
2. Installera och lägg till `WATCOM\binnt` i PATH
3. Ladda ner DOSBox-X från https://dosbox-x.com/

## Kompilera

```batch
wcl -0 -ms -i=include src\*.c -fe=game.exe
```

Flaggor:
- `-0` = 8086-kompatibel kod  
- `-ms` = Small memory model
- `-i=include` = Sökväg för header-filer
- `-fe=game.exe` = Output-filnamn

## Köra

```batch
dosbox-x game.exe
```

## Kontroller

### I spelet
| Tangent | Funktion |
|---------|----------|
| ← → | Gå vänster/höger |
| Mellanslag | Hoppa |
| P | Pausa |
| ESC | Avsluta |

### I menyer
| Tangent | Funktion |
|---------|----------|
| ↑ ↓ | Navigera |
| Enter | Välj |
| ESC | Avsluta |

## Features

- **Startmeny** - NEW GAME, CONTINUE, QUIT
- **Pausmeny** - RESUME, SAVE, MENU
- **Save/Load** - Spara till GAME.SAV (DOS-style binärfil)
- **Musik** - WAV-streaming via Sound Blaster
- **Ljudeffekter** - Mixas med musik i realtid
- **PC Speaker fallback** - Om Sound Blaster saknas

## Teknisk info

- **Grafik:** VGA Mode 13h (320×200, 256 färger)
- **Double buffering:** Flimmerfri rendering
- **Ljud:** Sound Blaster (port 0x220, IRQ 7, DMA 1)
- **Audio mixer:** Software mixing, double-buffered DMA
- **Sample rate:** 11025 Hz, 8-bit unsigned PCM
- **Målplattform:** MS-DOS / IBM PC-kompatibel
