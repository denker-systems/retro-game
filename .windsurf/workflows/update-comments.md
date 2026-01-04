---
name: update-comments
description: Uppdaterar en fil med detaljerade pedagogiska kommentarer enligt projektets standard
---

# Uppdatera Kommentarer

## Steg 1: Identifiera fil
Fråga användaren vilken fil som ska uppdateras, eller använd den aktiva filen i editorn.

## Steg 2: Analysera befintlig kod
Läs igenom filen och identifiera:
- Funktioner som saknar fullständiga kommentarer
- Kod som saknar inline-förklaringar
- Hårdvarurelaterade operationer som behöver förklaras

## Steg 3: Applicera kommentarstandard
Uppdatera filen enligt .windsurf/rules/comments.md:

### Filhuvud
Varje fil ska ha:
- SYFTE: Vad modulen gör
- HÅRDVARA: Vilken hårdvara/DOS-funktion som används
- BEROENDEN: Vilka andra moduler som inkluderas

### Funktionskommentarer
Varje funktion ska ha:
- PARAMETRAR: Beskrivning av varje parameter
- RETURNERAR: Vad som returneras
- FUNGERAR SÅ HÄR: Steg-för-steg förklaring

### Inline-kommentarer
- Förklara VARFÖR, inte bara VAD
- Beskriv minnesoperationer i detalj
- Förklara magiska tal och konstanter

## Steg 4: Verifiera
Visa en sammanfattning av ändringar som gjorts.
