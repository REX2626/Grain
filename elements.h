#include <SDL2/SDL.h>

// BASE

class Element{
    public:
        int x;
        int y;
        SDL_Colour colour;

        void render(SDL_Renderer* renderer, int size){
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

            SDL_Rect rect = {x * size, y * size, size, size};

            SDL_RenderFillRect(renderer, &rect);
        }

        void update(double deltaTime){
            y += 1;
        }

        void print(){
            std::cout << "Element of x: " << x << " y: " << y << "\n";
        }
};


// STATES

class Liquid: public Element{

};


class Solid: public Element{

};


class Gas: public Element{

};


// Intermediate

class MovableSolid: public Solid{

};


class ImmovableSolid: public Solid{

};


// Types

class Water: public Liquid{

};


class Sand: public MovableSolid{

};


class Stone: public ImmovableSolid{

};


class Smoke: public Gas{

};
