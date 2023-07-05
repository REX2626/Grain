#include <SDL2/SDL.h>

// BASE

class Element{
    public:
        int x;
        int y;
        SDL_Colour colour;

        void draw(SDL_Renderer*);

        void render(SDL_Renderer* renderer, int width, int height){
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

            SDL_Rect rect;
            rect.x = x * width;
            rect.y = y * height;
            rect.w = width;
            rect.h = height;

            SDL_RenderFillRect(renderer, &rect);
        }

        void update(double deltaTime){
            
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
