#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include "stack.c"

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const int CELL_SIZE = 24;
const int WALL_THICKNESS = 4;

const int MAZE_HEIGHT = SCREEN_HEIGHT / CELL_SIZE;
const int MAZE_WIDTH = SCREEN_WIDTH / CELL_SIZE;


struct maze {
    struct cell* board[45][80];
};


//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

struct cell* new_cell(int x, int y)
{
    struct cell *c = malloc(sizeof(struct cell));
    c->x = x;
    c->y = y;
    c->bottom = true;
    c->right = true;
    c->visited = false;
    c->on_path = false;
    return c;
}

struct maze *new_maze(int height, int width) {
    struct maze* M = malloc(sizeof(struct maze));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            M->board[i][j] = new_cell(j, i);
        }
    }
    return M;
}

void delete_maze(struct maze * maze)
{
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            free(maze->board[i][j]);
        }
    }
    free(maze);
}

struct cell* get_cell(struct cell* maze[MAZE_HEIGHT][MAZE_WIDTH], int x, int y) {
    if (x < 0 || y < 0 || x > MAZE_WIDTH - 1 || y > MAZE_HEIGHT - 1) {
        return NULL;
    }

    return maze[y][x];
}

int get_unvisited_neighbors(
    struct cell* maze[MAZE_HEIGHT][MAZE_WIDTH],
    int curr_x,
    int curr_y,
    struct cell** unvis_neighbours) {
    
    struct cell* north = NULL;
    struct cell* east = NULL;
    struct cell* south = NULL;
    struct cell* west = NULL;

    int count = 0;

    if (((north = get_cell(maze, curr_x, curr_y - 1)) != NULL) && !north->visited) {
        unvis_neighbours[count] = north;
        count += 1;
    }
    if (((east = get_cell(maze, curr_x + 1, curr_y)) != NULL) && !east->visited) {
        unvis_neighbours[count] = east;
        count += 1;
    }
    if (((south = get_cell(maze, curr_x, curr_y + 1)) != NULL) && !south->visited) {
        unvis_neighbours[count] = south;
        count += 1;
    }
    if (((west = get_cell(maze, curr_x - 1, curr_y)) != NULL) && !west->visited) {
        unvis_neighbours[count] = west;
        count += 1;
    }
    return count;
}

int get_unvisited_connected_neighbors(
    struct cell* maze[MAZE_HEIGHT][MAZE_WIDTH],
    int curr_x,
    int curr_y,
    struct cell** unvis_neighbours) {
    
    struct cell* curr = maze[curr_y][curr_x];
    struct cell* north = NULL;
    struct cell* east = NULL;
    struct cell* south = NULL;
    struct cell* west = NULL;

    int count = 0;

    if (((north = get_cell(maze, curr_x, curr_y - 1)) != NULL) && !north->visited && !north->bottom) {
        unvis_neighbours[count] = north;
        count += 1;
    }
    if (((east = get_cell(maze, curr_x + 1, curr_y)) != NULL) && !east->visited && !curr->right) {
        unvis_neighbours[count] = east;
        count += 1;
    }
    if (((south = get_cell(maze, curr_x, curr_y + 1)) != NULL) && !south->visited && !curr->bottom) {
        unvis_neighbours[count] = south;
        count += 1;
    }
    if (((west = get_cell(maze, curr_x - 1, curr_y)) != NULL) && !west->visited && !west->right) {
        unvis_neighbours[count] = west;
        count += 1;
    }
    return count;
}

int gen_rand(int min, int max) {
    return (rand() % (max - min + 1) + min);
}

void draw_maze(struct cell* maze[MAZE_HEIGHT][MAZE_WIDTH])
{
    SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255);

    //Clear screen
    SDL_RenderClear(gRenderer);


    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            struct cell *curr = maze[y][x];
            if (curr->visited && !curr->on_path) {
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255);
            } else if (curr->on_path && curr->visited) {
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 255);
            }
            else if (!curr->visited && !curr->on_path) {
                SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255);
            }
            // Draw cell
            SDL_Rect cell_rect;
            cell_rect.x = x * CELL_SIZE;
            cell_rect.y = y * CELL_SIZE;
            cell_rect.w = CELL_SIZE;
            cell_rect.h = CELL_SIZE;
            SDL_RenderFillRect(gRenderer, &cell_rect);
            // Draw walls
            SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 255);
            if (curr->right) {
                SDL_Rect wall_rect;
                wall_rect.x = (x * CELL_SIZE) + (CELL_SIZE - WALL_THICKNESS);
                wall_rect.y = y * CELL_SIZE;
                wall_rect.w = WALL_THICKNESS;
                wall_rect.h = CELL_SIZE;
                SDL_RenderFillRect(gRenderer, &wall_rect);
            }
            if (curr->bottom) {
                SDL_Rect wall_rect;
                wall_rect.x = x * CELL_SIZE;
                wall_rect.y = (y * CELL_SIZE) + (CELL_SIZE - WALL_THICKNESS);
                wall_rect.w = CELL_SIZE;
                wall_rect.h = WALL_THICKNESS;
                SDL_RenderFillRect(gRenderer, &wall_rect);   
            }
        }
    }

    //Update screen
    SDL_RenderPresent(gRenderer);
}

void generate_maze(struct cell* maze[MAZE_HEIGHT][MAZE_WIDTH]) {
    struct cell **stack = malloc(MAZE_HEIGHT * MAZE_WIDTH * sizeof(struct cell*));
    int stack_top = 0;
    struct cell* init_cell = maze[0][0];
    init_cell->visited = true;
    stack_push(init_cell, stack, &stack_top);
    while (stack_top != 0) {
        struct cell *curr_cell = stack_pop(stack, &stack_top);
        struct cell* u_neighbours[4] = {NULL, NULL, NULL, NULL};
        int u_count =  get_unvisited_neighbors(maze, curr_cell->x, curr_cell->y, u_neighbours);
        if (u_count > 0) {
            stack_push(curr_cell, stack, &stack_top);
            int index = gen_rand(0, u_count - 1);
            struct cell* neighbour = u_neighbours[index];
            if (curr_cell->y - 1 == neighbour->y) // North neighbour
            {
                neighbour->bottom = false;
            }
            else if (curr_cell->x + 1 == neighbour->x) // East neighbour
            {
                curr_cell->right = false;
            }
            else if (curr_cell->y + 1 == neighbour->y) // South neighbour
            {
                curr_cell->bottom = false;
            }
            else if (curr_cell->x - 1 == neighbour->x) // West neighbour
            {
                neighbour->right = false;
            }
            neighbour->visited = true;
            stack_push(neighbour, stack, &stack_top);
        }
        draw_maze(maze);
    }
    free(stack);
}

void solve_maze(struct cell* maze[MAZE_HEIGHT][MAZE_WIDTH])
{
    struct cell* start = maze[0][0];
    struct cell* end = maze[MAZE_HEIGHT - 1][MAZE_WIDTH - 1];

    struct cell **stack = malloc(MAZE_HEIGHT * MAZE_WIDTH * sizeof(struct cell*));
    struct cell* prev[MAZE_HEIGHT * MAZE_WIDTH];
    int stack_top = 0;
    stack_push(start, stack, &stack_top);
    while (stack_top != 0) {
        struct cell* current = stack_pop(stack, &stack_top);
        if (current == end) {
            break;
        }
        current->visited = true;
        struct cell* u_neighbours[4] = {NULL, NULL, NULL, NULL};
        int u_count =  get_unvisited_connected_neighbors(maze, current->x, current->y, u_neighbours);
        for (int i = 0; i < u_count; i++) {
            stack_push(u_neighbours[i], stack, &stack_top);
            prev[u_neighbours[i]->y* MAZE_WIDTH + u_neighbours[i]->x] = current;
        }
        draw_maze(maze);
    }
    SDL_RenderPresent(gRenderer);
    free(stack);
    struct cell* current = end;
    while (current != NULL) {
        current->on_path = true;
        current = prev[current->y * MAZE_WIDTH + current->x];
        draw_maze(maze);
    }
}

void reset_visited(struct cell* maze[MAZE_HEIGHT][MAZE_WIDTH])
{
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            maze[i][j]->visited = false;
        }
    }
}

bool init_sdl()
{
	//Initialization flag
	bool success = true;

	//Init. SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize!");
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow("Maze", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
		if (gWindow == NULL)
		{
			printf("Window could not be created!");
			success = false;
		}
		else 
		{
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created!");
                success = false;
            }
		}
	}
	return success;
}

void close_sdl()
{
	//Destroy window and renderer
    SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow(gWindow); // this method also takes care of gScreenSurface
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char const *argv[])
{
    // Main loop flag
    bool quit = false;
    init_sdl();
    struct maze* maze = new_maze(MAZE_HEIGHT, MAZE_WIDTH);
    bool visited_maze = false;
    while (!quit) 
    {
        // Event handler
        SDL_Event eventHandler;

        while(SDL_PollEvent(&eventHandler) != 0)
        {
            //User requests quit
            if(eventHandler.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        if (!visited_maze) {
            generate_maze(maze->board);
            reset_visited(maze->board);
            solve_maze(maze->board);
            visited_maze = true;
        }
    }
    delete_maze(maze);
    close_sdl();
    return 0;
}
