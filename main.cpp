#include <iostream>
#include <chrono>
#include <random>
#include <math.h>
#include <SDL2/SDL.h>

#include "elements.h"

using namespace std;

const int WIDTH = 800, HEIGHT = 800;
const int GRID_SIZE = 10;
const int GRID_WIDTH = WIDTH / GRID_SIZE, GRID_HEIGHT = HEIGHT / GRID_SIZE;

Element grid[GRID_WIDTH][GRID_HEIGHT];

int xMouse, yMouse;

// Pixel to grid square
int p_to_grid(int x){
    return floor(x / GRID_SIZE);
}

void place_element(){
    int x = p_to_grid(xMouse);
    int y = p_to_grid(yMouse);

    // Don't know how to shorten this yet
    Element element;
    element.x = x;
    element.y = y;
    element.colour.r = 255;
    element.colour.g = 165;
    element.colour.b = 0;
    element.colour.a = 255;

    grid[x][y] = element;
}

void draw(SDL_Renderer* renderer, double deltaTime){
    // Colours background black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Looping through all elements in the grid array and drawing each element
    for (int i = 0; i < GRID_WIDTH; i++){
        for (int j = 0; j < GRID_HEIGHT; j++){
            grid[i][j].render(renderer, GRID_SIZE);
        }
    }

    // Updates screen
    SDL_RenderPresent(renderer);
}

void update(double deltaTime){
    // Looping through all elements in the grid array and updating each element
    for (int i = 0; i < GRID_WIDTH; i++){
        for (int j = 0; j < GRID_HEIGHT; j++){
            grid[i][j].update(deltaTime);
        }
    }
}

int main(int argc, char* args[]){

    // Initial message & SDL version
    SDL_version version;
    SDL_GetVersion(&version);
    cout << "Initializing Grain (SDL version: " << (int) version.major << "." <<
            (int) version.minor << "." << (int) version.patch << ")" << endl;

    // Set up SDL
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

        // Place element if left click held down, as well as update mouse position
        if (SDL_GetMouseState(&xMouse, &yMouse) & SDL_BUTTON_LEFT){
                // Places an element at the mouse position
                place_element();
            }

        auto end = chrono::system_clock::now();
        chrono::duration<double> dt = end - start;
        deltaTime = dt.count();
        //cout << "FPS: " << (1.0 / deltaTime) << "\n";
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
