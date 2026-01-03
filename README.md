# Retro DOS Platformer

Ett minimalt Mario-liknande plattformsspel för MS-DOS, skrivet i C.

## Projektstruktur

```
src/
├── main.c      # Huvudprogram och spelloop
├── vga.c/h     # VGA Mode 13h grafik
├── input.c/h   # Tangentbordshantering  
├── player.c/h  # Spelarlogik och fysik
├── level.c/h   # Nivådata och kollisioner
└── types.h     # Gemensamma typer
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
wcl -0 -ms src\*.c -fe=game.exe
```

Flaggor:
- `-0` = 8086-kompatibel kod  
- `-ms` = Small memory model
- `-fe=game.exe` = Output-filnamn

## Köra

```batch
dosbox-x game.exe
```

## Kontroller

| Tangent | Funktion |
|---------|----------|
| ← → | Gå vänster/höger |
| Mellanslag | Hoppa |
| ESC | Avsluta |

## Teknisk info

- **Grafik:** VGA Mode 13h (320×200, 256 färger)
- **Double buffering:** Flimmerfri rendering
- **Målplattform:** MS-DOS / IBM PC-kompatibel
