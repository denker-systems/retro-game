---
trigger: glob
globs: ["*.c", "*.h"]
---

# C-kod Stilguide för DOS

<naming_conventions>
- Funktioner: module_action_thing (t.ex. vga_draw_rect)
- Typer: PascalCase (t.ex. Player, InputState)
- Konstanter: UPPER_SNAKE_CASE (t.ex. SCREEN_W)
- Variabler: snake_case (t.ex. player_x)
</naming_conventions>

<file_organization>
- Header guards i alla .h-filer
- Includes överst i filen
- static för interna funktioner
- Deklarera variabler i början av block (C89)
</file_organization>

<code_style>
- Inga // kommentarer (använd /* */)
- Deklarera loop-variabler före loopen
- Undvik float (använd fixed-point)
- Föredra early return
</code_style>

<example>
/* Korrekt C89-stil */
void player_update(Player *p) {
    int new_x;
    int collision;
    
    new_x = p->x + speed;
    collision = check_collision(new_x, p->y);
    
    if (collision < 0) {
        p->x = new_x;
    }
}
</example>
