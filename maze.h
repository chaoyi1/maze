#include <stdbool.h>

struct cell {
    int x;
    int y;
    bool bottom: 1;
    bool right: 1;
    bool visited: 1;
    bool on_path: 1;
};