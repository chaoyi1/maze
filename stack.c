#include <stdlib.h>
#include "maze.h"

struct cell* stack_pop(struct cell **stack, int *stack_top) {
    *stack_top -= 1;
    struct cell* result = stack[*stack_top];
    stack[*stack_top] = NULL;
    return result;
}

void stack_push(struct cell *item, struct cell **stack, int *stack_top) {
    stack[*stack_top] = item;
    *stack_top += 1;
    return;
}