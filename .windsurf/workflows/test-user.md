---
name: test-user
description: Testar användarens kunskap om DOS-spelprogrammering genom interaktiv diskussion
---

# Testa Din Kunskap

## Steg 1: Välj ämnesområde
Fråga användaren vilket område de vill testa:
1. **VGA-grafik** - Mode 13h, framebuffer, vsync
2. **Tangentbord** - Scan codes, interrupts, input
3. **Fysik** - Gravitation, kollision, hastighet
4. **C-programmering** - Pekare, structs, minneshantering
5. **DOS/BIOS** - Interrupts, minnesmodell, portar
6. **Slumpmässigt** - Blandade frågor från alla områden

## Steg 2: Ställ frågor
Ställ frågor i ökande svårighetsgrad. Anpassa baserat på svar.

### Exempel VGA-frågor:
- Varför använder vi 0xA0000000L för VGA-minnet?
- Vad är skillnaden mellan near och far pointers?
- Varför måste vi vänta på vsync?
- Hur beräknar man offset för pixel (x, y)?

### Exempel fysik-frågor:
- Varför är JUMP_FORCE negativ?
- Vad händer om vi tar bort terminal velocity?
- Förklara axis separation i kollisionshantering

### Exempel C-frågor:
- Varför skickar vi &player istället för player?
- Vad gör static framför en variabel?
- Varför använder vi far pointers för buffern?

## Steg 3: Ge feedback
Efter varje svar:
- Om rätt: Bekräfta och lägg eventuellt till extra info
- Om delvis rätt: Förklara vad som saknades
- Om fel: Förklara korrekt svar pedagogiskt, hänvisa till relevant kod

## Steg 4: Sammanfatta
Efter 5-10 frågor:
- Ge en sammanfattning av styrkor
- Föreslå områden att fördjupa sig i
- Rekommendera specifika övningar med /suggest-exercises

## Steg 5: Fortsätt eller avsluta
Fråga om användaren vill:
- Fortsätta med fler frågor?
- Byta ämnesområde?
- Få övningar för att förbättra svaga områden?
