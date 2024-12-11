#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define WIDTH 900 
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_GRAY 0xf1f1f1f1
#define COLOR_BACKGROUND 0x0f0f0f0f
#define COLOR_ORANGE 0xff763b
#define A_GRAVITY 0.3
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

// GEOMETRIC FUNCTIONS
double magnitude(double x, double y)
{
    return SDL_sqrt(x*x + y*y);
}

void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color)
{
    double low_x = circle.x - circle.radius;
    double low_y = circle.y - circle.radius;
    double high_x = circle.x + circle.radius;
    double high_y = circle.y + circle.radius;

    double radius_squared = circle.radius * circle.radius;

    for (int x = low_x; x < high_x; x++)
    {
        for (int y = low_y; y < high_y; y++)
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

void step(struct Circle* circle)
{
    // Calculate new Position
    circle->x += circle->v_x;
    circle->y += circle->v_y;
    circle->v_y += A_GRAVITY;

    // did the Ball exit the screen?
    // check if it exited on the right side
    if (circle->x + circle->radius > WIDTH)
    {
        circle->x = WIDTH-circle->radius;
        circle->v_x *= -1*DAMPENING;
    }
    // check if it exited on the left side
    if (circle->x - circle->radius < 0)
    {
        circle->x = circle->radius;
        circle->v_x *= -1*DAMPENING;
    }
    // check if it exited on the bottom side
    if (circle->y + circle->radius > HEIGHT)
    {
        circle->y = HEIGHT-circle->radius;
        // As it has touched the ground we do not need to add Gravity
        circle->v_y -= A_GRAVITY;
        // typecast int to reduce hopping beteen negative and positive
        circle->v_y = (int)circle->v_y*-1*DAMPENING;
        circle->v_x *= FRICTION;
    }
    // check if it exited on the top side
    if (circle->y - circle->radius < 0)
    {
        circle->y = circle->radius;
        circle->v_y *= -1;
        circle->v_x *= FRICTION;
    }
}

void collide(struct Circle* circle1, struct Circle* circle2)
{
    double distance_x = circle1->x - circle2->x;
    double distance_y = circle1->y - circle2->y;
    double distance = magnitude(distance_x, distance_y);
    if (distance < circle1->radius + circle2->radius)
    {
        // Correct possible overlap before adding velocity
        double overlap = distance - (circle1->radius + circle2-> radius);
        double dir_x = circle2->x - circle1->x;
        double dir_y = circle2->y - circle1->y;
        dir_x /= magnitude(dir_x,dir_y);
        dir_y /= magnitude(dir_x,dir_y);
        dir_x *= overlap * 0.5;
        dir_y *= overlap * 0.5;
        circle1->x += dir_x;
        circle1->y += dir_y;
        circle2->x -= dir_x;
        circle2->y -= dir_y;

        // Extract from paper: https://dipamsen.github.io/notebook/collisions.pdf

        double m1 = (circle1->radius/20)*(circle1->radius/20);
        double m2 = (circle2->radius/20)*(circle2->radius/20);
        double v1[] = {circle1->v_x,circle1->v_y};
        double v2[] = {circle2->v_x,circle2->v_y};
        double x1[] = {circle1->x,circle1->y};
        double x2[] = {circle2->x,circle2->y};

        double x2x1[] = {x2[0]-x1[0],x2[1]-x1[1]};
        double x1x2[] = {x1[0]-x2[0],x1[1]-x2[1]};
        double v2v1[] = {v2[0]-v1[0],v2[1]-v1[1]};
        double v1v2[] = {v1[0]-v2[0],v1[1]-v2[1]};
        double v2v1_x2x1_dot = v2v1[0]*x2x1[0] + v2v1[1]*x2x1[1];
        double v1v2_x1x2_dot = v1v2[0]*x1x2[0] + v1v2[1]*x1x2[1];

        double numerator1 = 2*m2*v2v1_x2x1_dot;
        double numerator2 = 2*m1*v1v2_x1x2_dot;
        double denominator = (m1+m2)*distance;

        double deltaVA[] = {(numerator1/denominator)*x2x1[0],(numerator1/denominator)*x2x1[1]};
        double deltaVB[] = {(numerator2/denominator)*x1x2[0],(numerator2/denominator)*x1x2[1]};

        circle1->v_x = deltaVA[0]/200; // Dividing by  200 (trial and error) to get the best effect ?
        circle1->v_y = deltaVA[1]/200; // Dividing by  200 (trial and error) to get the best effect ?
        circle2->v_x = deltaVB[0]/200; // Dividing by  200 (trial and error) to get the best effect ?
        circle2->v_y = deltaVB[1]/200; // Dividing by  200 (trial and error) to get the best effect ?
    }
}

void stepAll(struct Circle* circles[], int circles_length)
{  
    for (int i = 0; i < circles_length; i++)
    {
        for (int j = i + 1; j < circles_length; j++)
        {
            if (i == j) continue;
            collide(circles[i],circles[j]);
        }
    }
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Bouncy Balls",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    struct Circle circle = {300, 200, 80, 15, 15};
    struct Circle circle2 = {100, 200, 50, -15, 10};
    struct Circle circle3 = {50, 300, 60, -10, 15};
    struct Circle* circles[] = {&circle,&circle2, &circle3};
    int circles_length = sizeof(circles) / sizeof(*circles); // Divide total memory length of array by mem. length of one item

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
                    for (int i = 0; i < circles_length; i++)
                    {
                        circles[i]->v_x += circles[i]->v_x<0 ? -5 : 5;
                        circles[i]->v_y = -20;
                    }
            }
        }

        SDL_FillRect(surface,&erase_rect,COLOR_BACKGROUND);

        stepAll(circles, circles_length);

        FillCircle(surface,circle, COLOR_WHITE);
        step(&circle);

        FillCircle(surface,circle2, COLOR_BLACK);
        step(&circle2);

        FillCircle(surface,circle3, COLOR_ORANGE);
        step(&circle3);

        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }
    return 0;
}