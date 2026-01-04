---
name: compile
description: Kompilerar spelet med Open Watcom och startar i DOSBox-X
---

# Kompilera och Kör

## Steg 1: Sätt upp miljövariabler
// turbo
Kör följande för att sätta upp Open Watcom:
```powershell
$env:WATCOM = "C:\WATCOM"
$env:PATH = "$env:WATCOM\binnt64;$env:WATCOM\binnt;$env:PATH"
```

## Steg 2: Kompilera
// turbo
Kompilera alla källfiler:
```powershell
C:\WATCOM\binnt64\wcl.exe -0 -ms -i=include src\main.c src\vga.c src\input.c src\player.c src\level.c -fe=game
```

Flaggor:
- `-0` = 8086-kompatibel kod
- `-ms` = Small memory model
- `-i=include` = Header-sökväg
- `-fe=game` = Output: game.exe

## Steg 3: Kontrollera resultat
Verifiera att game.exe skapades och visa eventuella varningar.

## Steg 4: Kör i DOSBox-X (valfritt)
Fråga användaren om de vill starta spelet direkt i DOSBox-X.

Om ja:
```powershell
Start-Process "C:\DOSBox-X\dosbox-x.exe" -ArgumentList "C:\Users\Calle\Documents\GitHub\retro-game\game.exe"
```

## Vanliga fel

### "wcl is not recognized"
Open Watcom är inte installerat eller inte i PATH.
Lösning: Installera Open Watcom eller sätt miljövariabler manuellt.

### Varningar om far pointers
Normalt för DOS-programmering - kan ignoreras.

### "Unable to open '.exe'"
Använd `-fe=game` utan .exe i slutet.
