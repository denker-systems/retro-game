---
trigger: always_on
---

# DOS Hårdvarubegränsningar

<memory>
- 640 KB konventionellt minne
- Använd far-pekare för videominne
- Allokera med _fmalloc(), frigör med _ffree()
- Kontrollera alltid allokeringsresultat
</memory>

<vga_mode_13h>
- Upplösning: 320x200 pixlar
- Färger: 256 (8-bit palette)
- Framebuffer: 0xA0000 (64000 bytes)
- Använd dubbelbuffring mot flimmer
- Synka med vsync (port 0x3DA)
</vga_mode_13h>

<performance>
- Undvik floating point (ingen FPU)
- Använd fixed-point matematik vid behov
- Förberäkna lookup-tabeller
- Minimera minneskopieringar
- Inline kritiska loopar i assembly vid behov
</performance>

<interrupts>
- INT 10h: BIOS video services
- INT 16h: Tangentbord
- INT 21h: DOS services
- Använd union REGS och int86() för anrop
</interrupts>
