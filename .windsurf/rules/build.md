---
trigger: always_on
---

# Kompilering och Testning

<open_watcom>
Kompilera med:
wcl -0 -ms src\*.c -fe=game.exe

Flaggor:
- -0 = 8086-kompatibel kod
- -ms = Small memory model
- -fe= = Output-filnamn
</open_watcom>

<testing>
- Testa alltid i DOSBox-X
- Kontrollera minnesläckor
- Verifiera att vsync fungerar (inget tearing)
- Testa på olika CPU-hastigheter i DOSBox
</testing>

<common_pitfalls>
1. Glömma far-pekare för videominne
2. Inte vänta på vsync (orsakar tearing)
3. Använda float utan FPU
4. Glömma återställa textläge vid avslut
5. Minnesläckor från ej frigjorda allokeringar
6. Använda // kommentarer (ej C89)
7. Deklarera variabler mitt i block
</common_pitfalls>
