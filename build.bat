@echo off
REM Kompilerar spelet med Open Watcom
set WATCOM=C:\WATCOM
set PATH=%WATCOM%\binnt64;%WATCOM%\binnt;%PATH%
set INCLUDE=%WATCOM%\h

echo Kompilerar...
wcl -0 -ms -i=include -i=%WATCOM%\h src\main.c src\vga.c src\input.c src\player.c src\level.c src\menu.c src\game.c src\sound.c -fe=game

if exist game.exe (
    echo.
    echo === Kompilering lyckades! ===
    echo Kör "play.bat" för att starta spelet.
) else (
    echo.
    echo === Kompilering misslyckades ===
)
