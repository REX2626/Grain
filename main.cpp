#include <iostream>
#include <chrono>
#include <random>
#include <math.h>
#include <SDL2/SDL.h>

#include "elements.h"

using namespace std;

const int WIDTH = 800, HEIGHT = 600;
const int GRID_RATIO = 10;
const int GRID_WIDTH = WIDTH / GRID_RATIO, GRID_HEIGHT = HEIGHT / GRID_RATIO;

Element grid[GRID_WIDTH][GRID_HEIGHT];

void draw(SDL_Renderer *renderer, double deltaTime){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    SDL_RenderPresent(renderer);
}

void update(double deltaTime){

}

int main(int argc, char *args[]){

    // Set up
    if (SDL_Init(SDL_INIT_VIDEO||SDL_INIT_EVENTS) < 0){
        cout << "SDL could not initialize: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Grain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL){
        cout << "Could not create window: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL){
        cout << "Could not create renderer: " << SDL_GetError() << endl;
        return 1;
    }

    // Set up Main loop
    double deltaTime = 0;
    bool running = true;

    SDL_Event event;

    // Main loop
    while (running){

        auto start = chrono::system_clock::now();

        update(deltaTime);
        draw(renderer, deltaTime);

        // Handle events
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running = false;
                break;
            }
        }

        auto end = chrono::system_clock::now();
        chrono::duration<double> dt = end - start;
        deltaTime = dt.count();
        cout << "FPS: " << (1.0 / deltaTime) << "\n";
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
