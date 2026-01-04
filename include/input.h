/*
 * input.h - Tangentbordshantering
 */
#ifndef INPUT_H
#define INPUT_H

typedef struct {
    int left;
    int right;
    int jump;
    int quit;
} InputState;

void input_update(InputState *state);

#endif
