# Windsurf Rules - DOS Game Development in C

## Project Context
This is an educational MS-DOS game project written in C, targeting IBM PC compatible systems from the 1985-1995 era.

## Language & Communication
- Communicate in Swedish
- Explain concepts thoroughly for learning purposes
- Reference historical context when relevant

## Code Style

### General
- Write C89/C90 compliant code (ANSI C)
- Use Open Watcom compatible syntax
- Prefer minimal implementations
- Keep functions small and focused

### Naming Conventions
```c
// Functions: module_action_thing
void vga_draw_rect(int x, int y, int w, int h);
void player_update(Player *p);

// Types: PascalCase
typedef struct { int x, y; } Player;

// Constants: UPPER_SNAKE_CASE
#define SCREEN_W 320
#define PLAYER_SPEED 3

// Variables: snake_case
int player_x;
int frame_count;
```

### File Organization
- One module per .c/.h pair
- Header guards in all .h files
- Includes at top of file
- Static functions for internal use

## DOS/Hardware Constraints

### Memory
- 640 KB conventional memory limit
- Use `far` pointers for video memory
- Allocate buffers with `_fmalloc()`
- Always free allocated memory

### Graphics (VGA Mode 13h)
- Resolution: 320×200, 256 colors
- Framebuffer at 0xA0000
- Use double buffering to prevent flicker
- Sync with vsync (port 0x3DA)

### Performance
- Avoid floating point (use fixed-point math)
- Inline critical loops in assembly if needed
- Minimize memory copies
- Pre-calculate lookup tables

## Architecture Principles

### Module Separation
```
src/
├── main.c      # Game loop, initialization
├── vga.c/h     # All VGA graphics operations
├── input.c/h   # Keyboard handling
├── player.c/h  # Player state and physics
├── level.c/h   # World data and collision
└── types.h     # Shared types and constants
```

### Dependencies
- Modules should depend on abstractions
- vga.c should not know about player.c
- player.c calls vga_draw_rect(), not raw VGA access

## Teaching Approach

When explaining code:
1. Explain the hardware/DOS concept first
2. Show the minimal C implementation
3. Describe why it works that way
4. Mention historical context if interesting

Key concepts to understand:
- **Interrupts** - How DOS/BIOS services work (INT 10h, 21h)
- **Segmented memory** - segment:offset addressing
- **Real mode** - Direct hardware access
- **VGA programming** - Mode setting, framebuffer, palette
- **Game loop** - Input → Update → Render cycle

## Compiler & Build

### Open Watcom
```batch
wcl -0 -ms src\*.c -fe=game.exe
```
- `-0` = 8086 compatible code
- `-ms` = Small memory model
- `-fe=` = Output filename

### Testing
- Always test in DOSBox-X
- Check for memory leaks
- Verify vsync is working (no tearing)

## Common Pitfalls

1. **Forgetting `far` pointers** for video memory
2. **Not waiting for vsync** causing screen tearing
3. **Using float** on systems without FPU
4. **Forgetting to restore text mode** on exit
5. **Memory leaks** from unfreed allocations

## Reference Resources

- Ralf Brown's Interrupt List
- PCGPE (PC Game Programmer's Encyclopedia)
- Abrash's Graphics Programming Black Book
- VGA hardware documentation
