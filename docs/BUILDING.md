# Kompilering och Körning

## Krav

### Open Watcom 2.0
C-kompilator för DOS-programmering.

**Installation:**
```powershell
# Ladda ner
curl.exe -L -o "$env:TEMP\open-watcom-setup.exe" `
  "https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/open-watcom-2_0-c-win-x64.exe"

# Installera (GUI)
Start-Process "$env:TEMP\open-watcom-setup.exe"
```

**Viktigt vid installation:**
- Installera till `C:\WATCOM`
- Bocka i **16-bit x86 DOS**
- Bocka i **Add to PATH**

### DOSBox-X
DOS-emulator för att köra spelet.

**Installation:**
```powershell
winget install joncampbell123.DOSBox-X
```

## Kompilera

### Snabbkommando
```powershell
$env:WATCOM = "C:\WATCOM"
$env:PATH = "$env:WATCOM\binnt64;$env:WATCOM\binnt;$env:PATH"
C:\WATCOM\binnt64\wcl.exe -0 -ms -i=include src\main.c src\vga.c src\input.c src\player.c src\level.c -fe=game
```

### Flaggor förklarade

| Flagga | Betydelse |
|--------|-----------|
| `-0` | Generera 8086-kompatibel kod (fungerar på alla x86) |
| `-ms` | Small memory model (kod+data < 64KB vardera) |
| `-i=include` | Sökväg till header-filer |
| `-fe=game` | Output-fil (game.exe) |

### Andra användbara flaggor

| Flagga | Betydelse |
|--------|-----------|
| `-d2` | Full debug-info |
| `-ox` | Maximal optimering |
| `-w4` | Alla varningar |
| `-we` | Varningar som fel |

## Köra

### Direkt start
```powershell
Start-Process "C:\DOSBox-X\dosbox-x.exe" -ArgumentList "game.exe"
```

### Manuellt i DOSBox-X
```
Z:\>mount c C:\Users\...\retro-game
Z:\>c:
C:\>game.exe
```

## Felsökning

### "wcl is not recognized"
Open Watcom är inte i PATH. Sätt miljövariabler:
```powershell
$env:WATCOM = "C:\WATCOM"
$env:PATH = "$env:WATCOM\binnt64;$env:PATH"
```

### "Unable to open '.exe'"
Syntaxfel i kompileringskommandot. Använd `-fe=game` (utan .exe).

### Spelet startar inte i DOSBox
Kontrollera att:
1. game.exe finns i mappen
2. Rätt sökväg är monterad
3. Du är på rätt drive (C: eller D:)

### Skärmen blinkar/flimrar
Vsync fungerar inte korrekt. Kontrollera `vga_vsync()` anropas före `vga_flip()`.

## Makefile (valfritt)

Skapa `Makefile` för enklare kompilering:

```makefile
CC = wcl
CFLAGS = -0 -ms -i=include
SRCS = src\main.c src\vga.c src\input.c src\player.c src\level.c
TARGET = game

all: $(TARGET).exe

$(TARGET).exe: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -fe=$(TARGET)

clean:
	del *.obj *.exe *.err
```

Kör med: `wmake` (Open Watcom's make)
