---
name: suggest-exercises
description: Föreslår övningar för att träna DOS-spelprogrammering i C
---

# Föreslå Övningar

## Steg 1: Bedöm kunskapsnivå
Fråga användaren var de befinner sig:
1. Nybörjare - förstår grunderna i C
2. Mellanliggande - kan läsa och förstå koden
3. Avancerad - vill ha utmaningar

## Steg 2: Föreslå övningar baserat på nivå

### Nybörjare-övningar
- **Ändra färger**: Byt färg på spelaren, plattformar, eller bakgrund
- **Ändra fysik**: Justera GRAVITY, JUMP_FORCE, MOVE_SPEED
- **Flytta plattformar**: Ändra plattformarnas positioner i level.c
- **Lägg till ny plattform**: Utöka platforms-arrayen

### Mellanliggande-övningar
- **Animerad spelare**: Rita olika sprites baserat på rörelseriktning
- **Samlarobjekt**: Lägg till mynt/stjärnor som spelaren kan plocka upp
- **Enkel fiende**: En fiende som rör sig fram och tillbaka
- **Poängsystem**: Visa poäng på skärmen
- **Rörlig plattform**: Plattform som åker horisontellt

### Avancerade övningar
- **Sprite-rendering**: Ladda och visa bitmaps istället för rektanglar
- **Scroll-värld**: Kamera som följer spelaren i en större nivå
- **Ljudeffekter**: PC Speaker eller Sound Blaster-ljud
- **Nivåladare**: Ladda nivådata från en fil
- **Partikelsystem**: Damm, explosioner, etc.

## Steg 3: Ge detaljerad vägledning
När användaren väljer en övning:
1. Förklara konceptet bakom övningen
2. Visa vilka filer som behöver ändras
3. Ge steg-för-steg instruktioner
4. Erbjud hjälp om de kör fast

## Steg 4: Uppföljning
Fråga om de vill ha:
- Mer detaljerad hjälp med övningen?
- Se en exempellösning?
- Prova en annan övning?
