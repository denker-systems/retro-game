@echo off
REM Testar Sound Blaster i DOSBox-X

echo [autoexec] > "%TEMP%\sbtest.conf"
echo SET BLASTER=A220 I7 D1 T4 >> "%TEMP%\sbtest.conf"
echo mount C "%~dp0" >> "%TEMP%\sbtest.conf"
echo C: >> "%TEMP%\sbtest.conf"
echo sbtest.exe >> "%TEMP%\sbtest.conf"
echo pause >> "%TEMP%\sbtest.conf"

start "" "C:\DOSBox-X\dosbox-x.exe" -conf "%TEMP%\sbtest.conf"
