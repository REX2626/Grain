#include <iostream>
#include <chrono>
#include <random>
#include <math.h>
#include <SDL2/SDL.h>

using namespace std;

const int FPS = 60;
const int WIDTH = 1000, HEIGHT = 600;
const int GRID_SIZE = 8;
const int MAX_PLACE_SIZE = 10;
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
    return x / GRID_SIZE;
}

void place_element(int x0, int y0){
    if (!grid.inBounds(x0, y0)){return;} // Only place element in bounds

    int dx, dy, x, y;
    Element *placedElement;
    for (dx=-placeSize; dx<=placeSize; ++dx) // place inside circle, radius ~placeSize
    for (dy=-placeSize; dy<=placeSize; ++dy) {
        x = x0 + dx;
        y = y0 + dy;
        if (!grid.inBounds(x, y)) {continue;}
        if (dx*dx + dy*dy > placeSize*placeSize + 1) {continue;} // dont place if outside circle around cursor

        switch (selectedElement) {
            case 0: placedElement = new Stone(x, y); break;
            case 1: placedElement = new Sand(x, y); break;
            case 2: placedElement = new Water(x, y); break;
            case 3: placedElement = new Dirt(x, y); break;
            case 4: placedElement = new Coal(x, y); break;
        }
        if (grid.isFull(x, y) && grid.getPtr(x, y)->tag == placedElement->tag) {continue;} // don't overwrite same element
        grid.set(x, y, placedElement);
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

void remove_element() {
    int x0 = p_to_grid(xMouse);
    int y0 = p_to_grid(yMouse);

    int dx, dy, x, y;
    for (dx=-placeSize; dx<=placeSize; ++dx)
    for (dy=-placeSize; dy<=placeSize; ++dy) {
        x = x0 + dx;
        y = y0 + dy;
        if (!grid.inBounds(x, y)) {continue;} // Only remove element in bounds
        if ((dx*dx) + (dy*dy) > placeSize*placeSize + 1) {continue;} // Only remove element inside circle around cursor
        grid.set(x, y, nullptr);
    }
}

void draw(SDL_Renderer* renderer, double deltaTime){
    // Colours background black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Looping through all elements in the grid array and drawing each element
    for (int x = 0; x < GRID_WIDTH; x++){
        for (int y = 0; y < GRID_HEIGHT; y++){
            if (grid.isFull(x, y)){
                grid.getPtr(x, y)->render(renderer, GRID_SIZE);
            }
        }
    }

    // Updates screen
    SDL_RenderPresent(renderer);
}

void update(double deltaTime){
    // grid.copy(); // Ensures elements can only be updated once
    // Looping through all elements in the grid array and updating each element
    int x, y;
    for (y=0; y<GRID_HEIGHT; ++y)
    for (x=0; x<GRID_WIDTH; ++x) {
        if (grid.isEmpty(x, y)) {continue;}
        grid.getPtr(x, y)->updated = false;

    }
    Element *elem;
    bool rand_choice = rand()%2;
    int x_iter;
    if (rand_choice) {
        x = 0;
        x_iter = 1;
    } else {
        x = GRID_WIDTH-1;
        x_iter = -1;
    }
    for (; x >= 0 && x < GRID_WIDTH; x += x_iter) {
        for (y = GRID_HEIGHT-1; y >= 0; y--) { // From bottom to top
            if (grid.isEmpty(x, y)) {continue;}
            elem = grid.getPtr(x, y);
            if (elem->updated) {
                continue;
            }
            elem->update(deltaTime);
            elem->updated = true;
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

                    case SDLK_UP:
                        if (placeSize < MAX_PLACE_SIZE){placeSize++;}
                        break;

                    case SDLK_DOWN:
                        if (placeSize > 0){placeSize--;}
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
                    if (placeSize < MAX_PLACE_SIZE){
                        placeSize++;
                    }
                } else if (event.wheel.y < 0){ // Scroll down
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
