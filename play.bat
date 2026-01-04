@echo off
REM Startar spelet i DOSBox-X med Sound Blaster-stöd
REM Skapar en temporär .conf fil för att sätta BLASTER-miljövariabeln

echo [autoexec] > "%TEMP%\retrogame.conf"
echo SET BLASTER=A220 I7 D1 T4 >> "%TEMP%\retrogame.conf"
echo mount C "%~dp0" >> "%TEMP%\retrogame.conf"
echo C: >> "%TEMP%\retrogame.conf"
echo game.exe >> "%TEMP%\retrogame.conf"
echo exit >> "%TEMP%\retrogame.conf"

start "" "C:\DOSBox-X\dosbox-x.exe" -conf "%TEMP%\retrogame.conf"
