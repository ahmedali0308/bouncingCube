#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define WIDTH 900 
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_GRAY 0xf1f1f1f1
#define COLOR_BACKGROUND 0x0f0f0f0f
#define COLOR_TRAJECTORY 0xff763b
#define A_GRAVITY 0.8
#define DAMPENING 0.7
#define FRICTION 0.95
#define TRAJECTORY_LENGTH 100
#define TRAJECTORY_WIDTH 10

struct Circle
{
    double x;
    double y;
    double radius;
    double v_x;
    double v_y;
};

void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color)
{
    double low_x = circle.x - circle.radius;
    double low_y = circle.y - circle.radius;
    double high_x = circle.x + circle.radius;
    double high_y = circle.y + circle.radius;

    double radius_squared = circle.radius * circle.radius;

    for (double x = low_x; x < high_x; x++)
    {
        for (double y = low_y; y < high_y; y++)
        {
            double center_distance_squared = (x-circle.x)*(x-circle.x) + (y-circle.y)*(y-circle.y);
            if (center_distance_squared < radius_squared)
            {
                SDL_Rect pixel = (SDL_Rect){x,y,1,1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

void FillTrajectory(SDL_Surface* surface, struct Circle trajectory[TRAJECTORY_LENGTH], int current_trajectory_index, Uint32 color)
{
    for (int i = 0; i < current_trajectory_index; i++)
    {
        trajectory[i].radius = TRAJECTORY_WIDTH*i/100;
        FillCircle(surface, trajectory[i], color);
    }
}

void step(struct Circle* circle)
{
    // How to calculate the new position
    circle->x += circle->v_x;
    circle->y += circle->v_y;
    circle->v_y += A_GRAVITY;

    // did the Ball exit the screen?
    if (circle->x + circle->radius > WIDTH)
    {
        circle->x = WIDTH-circle->radius;
        circle->v_x *= -1*DAMPENING;
    }
    if (circle->y + circle->radius > HEIGHT)
    {
        circle->y = HEIGHT-circle->radius;
        circle->v_y *= -1*DAMPENING;
        circle->v_x *= FRICTION;
        // To avoid endless jumping and rolling, implement a Threshold under which V becomes 0
        if (circle->v_y > 0)
        {
            if (circle->v_y < 4) circle->v_y = 0;
        }
        else
        {
            if (circle->v_y > -4) circle->v_y = 0;
        }

        if (circle->v_x > 0)
        {
            if (circle->v_x < 1) circle->v_x = 0;
        }
        else
        {
            if (circle->v_x > -1) circle->v_x = 0;
        }
    }
    if (circle->x - circle->radius < 0)
    {
        circle->x = circle->radius;
        circle->v_x *= -1*DAMPENING;
    }
    if (circle->y - circle->radius < 0)
    {
        circle->y = circle->radius;
        circle->v_y *= -1*DAMPENING;
    }
}

void UpdateTrajectory(struct Circle trajectory[TRAJECTORY_LENGTH], struct Circle circle, int current_index)
{
    if (current_index >= TRAJECTORY_LENGTH)
    {
        // shift array - write the circle at the end of the array
        for (int i = 1; i<TRAJECTORY_LENGTH;i++)
        {
            trajectory[i-1] = trajectory[i];
        }
        trajectory[TRAJECTORY_LENGTH-1] = circle;
    }
    else
    {
        trajectory[current_index] = circle;
    }
}

int main()
{
    printf("Hello Bouncy Ball\n");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Bouncy Ball",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    struct Circle circle = {200, 200, 80, 50, 50};
    struct Circle trajectory[TRAJECTORY_LENGTH];
    int current_trajectory_index = 0;

    SDL_Rect erase_rect = (SDL_Rect){0,0,WIDTH,HEIGHT};
    SDL_UpdateWindowSurface(window);

    SDL_Event event;
    int simulation_running = 1;
    while (simulation_running)
    {
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                simulation_running = 0;
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                    circle.v_x += circle.v_x<0 ? -5 : 5;
                    circle.v_y -= 20;
            }
        }

        SDL_FillRect(surface,&erase_rect,COLOR_BACKGROUND);
        FillTrajectory(surface,trajectory,current_trajectory_index,COLOR_TRAJECTORY);
        FillCircle(surface,circle, COLOR_WHITE);
        
        step(&circle);
        UpdateTrajectory(trajectory,circle,current_trajectory_index);
        current_trajectory_index = current_trajectory_index >= TRAJECTORY_LENGTH ? TRAJECTORY_LENGTH : current_trajectory_index+1;
        SDL_UpdateWindowSurface(window);
        SDL_Delay(20);
    }
    return 0;
}