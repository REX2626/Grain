#include <iostream>
#include <chrono>
#include <random>
#include <math.h>
#include <SDL2/SDL.h>

using namespace std;

const int FPS = 60;
const int WIDTH = 800, HEIGHT = 600;
const int GRID_SIZE = 10;
const int GRID_WIDTH = WIDTH / GRID_SIZE, GRID_HEIGHT = HEIGHT / GRID_SIZE;

#include "base_element.h"
#include "grid.h"
Grid grid;
#include "elements.h"


int xMouse, yMouse;
int oldXMouse, oldYMouse;
int placeSize = 0;

int selectedElement = 0;

// Pixel to grid square
int p_to_grid(int x){
    return floor(x / GRID_SIZE);
}

void place_element(int x, int y){
    if (!grid.inBounds(x, y)){return;} // Only place element in bounds

    // Place a square of side length 2*placeSize + 1
    for (int i = -placeSize; i <= placeSize; i++){
        for (int j = -placeSize; j <= placeSize; j++){
            if (!grid.inBounds(x + i, y + j)){continue;}
            switch (selectedElement){
                case 0: // Stone
                    if (grid.isEmpty(x + i, y + j) || grid.get(x + i, y + j).tag != "stone"){
                        grid.set(x + i, y + j, new Stone(x + i, y + j));
                    }
                    break;
                case 1: // Sand
                    if (grid.isEmpty(x + i, y + j) || grid.get(x + i, y + j).tag != "sand"){
                        grid.set(x + i, y + j, new Sand(x + i, y + j));
                    }
                    break;
                case 2: // Water
                    if (grid.isEmpty(x + i, y + j) || grid.get(x + i, y + j).tag != "water"){
                        grid.set(x + i, y + j, new Water(x + i, y + j));
                    }
                    break;
                case 3: // Dirt
                    if (grid.isEmpty(x + i, y + j) || grid.get(x + i, y + j).tag != "dirt"){
                        grid.set(x + i, y + j, new Dirt(x + i, y + j));
                    }
                    break;
                case 4: // Coal
                    if (grid.isEmpty(x + i, y + j) || grid.get(x + i, y + j).tag != "coal"){
                        grid.set(x + i, y + j, new Coal(x + i, y + j));
                    }
                    break;
            }
        }
    }
}

void placeBetween(int startX, int startY, int endX, int endY){
    int xDirection = 1;
    if (endX < startX){xDirection = -1;}

    int yDirection = 1;
    if (endY < startY){yDirection = -1;}

    if (endX == startX){ // Moving vertically
        for (int i = 0; i <= abs(endY - startY); i++){
            place_element(startX, startY + i * yDirection);
        }

    }else { // Not moving vertically
        float gradient = (float)(endY - startY) / (float)(endX - startX);

        if (abs(gradient) > 1){
            gradient = 1 / gradient;
            for (int i = 0; i <= abs(endY - startY); i++){
                place_element(startX + round(i * gradient) * yDirection, startY + i * yDirection);
            }
        }
        else{
            for (int i = 0; i <= abs(endX - startX); i++){
                place_element(startX + i * xDirection, startY + round(i * gradient) * xDirection);
            }
        }

    }
}

void remove_element(){
    int x = p_to_grid(xMouse);
    int y = p_to_grid(yMouse);

    if (!grid.inBounds(x, y)){return;} // Only remove element in bounds

    grid.set(x, y, nullptr);
}

void draw(SDL_Renderer* renderer, double deltaTime){
    // Colours background black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Looping through all elements in the grid array and drawing each element
    for (int x = 0; x < GRID_WIDTH; x++){
        for (int y = 0; y < GRID_HEIGHT; y++){
            if (grid.isFull(x, y)){
                grid.get(x, y).render(renderer, GRID_SIZE);
            }
        }
    }

    // Updates screen
    SDL_RenderPresent(renderer);
}

void update(double deltaTime){
    grid.copy(); // Ensures elements can only be updated once
    // Looping through all elements in the grid array and updating each element
    for (int x = 0; x < GRID_WIDTH; x++){
        for (int y = GRID_HEIGHT-1; y >= 0; y--){ // From bottom to top
            if (grid.isFullCopy(x, y)){
                (*grid.getPtrCopy(x, y)).update(deltaTime); // Have to use pointer, as Element will force Element methods
            }
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

    // Fill grid with null pointers
    for (int x = 0; x < GRID_WIDTH; x++){
        for (int y = 0; y < GRID_HEIGHT; y++){
            grid.set(x, y, nullptr);
        }
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

            else if (event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym){

                    case SDLK_1:
                        selectedElement = 0;
                        break;

                    case SDLK_2:
                        selectedElement = 1;
                        break;

                    case SDLK_3:
                        selectedElement = 2;
                        break;

                    case SDLK_4:
                        selectedElement = 3;
                        break;

                    case SDLK_5:
                        selectedElement = 4;
                        break;

                    case SDLK_r:
                        grid.reset();
                        break;

                    case SDLK_p:
                        SDL_GetMouseState(&xMouse, &yMouse);
                        place_element(p_to_grid(xMouse), p_to_grid(yMouse));
                        break;

                }
            }

            else if (event.type == SDL_MOUSEWHEEL){
                if (event.wheel.y > 0){ // Scroll up
                    if (placeSize < 5){
                        placeSize++;
                    }
                }
                else if (event.wheel.y < 0){ // Scroll down
                    if (placeSize > 0){
                        placeSize--;
                    }
                }
            }
        }

        // Place element if left click held down, as well as update mouse position
        if (SDL_GetMouseState(&xMouse, &yMouse) & SDL_BUTTON_LEFT){
                // Places the selected element at the mouse position
                placeBetween(p_to_grid(oldXMouse), p_to_grid(oldYMouse), p_to_grid(xMouse), p_to_grid(yMouse));
        }
        // If right click held down, delete element at position
        else if (SDL_GetMouseState(&xMouse, &yMouse) & SDL_BUTTON_X1){
            remove_element();
        }

        oldXMouse = xMouse;
        oldYMouse = yMouse;

        auto end = chrono::system_clock::now();
        chrono::duration<double> dt = end - start;
        deltaTime = dt.count();
        if (deltaTime < 1.0/FPS){
            SDL_Delay(1000 * (1.0/FPS - deltaTime)); // Max framerate is FPS, ensures elements fall slow enough
        }
        //cout << "FPS: " << (1.0 / deltaTime) << "\n";
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
