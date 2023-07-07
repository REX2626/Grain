#include <SDL2/SDL.h>

using namespace std;

// BASE

class Element{
    public:
        int x;
        int y;
        SDL_Colour colour = {106, 13, 173}; // Purple (this should be overridden by child class)

        Element(int x, int y): x(x), y(y){

        }

        void render(SDL_Renderer* renderer, int size){
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

            SDL_Rect rect = {x * size, y * size, size, size};

            SDL_RenderFillRect(renderer, &rect);
        }

        virtual void update(double deltaTime){ // virtual means this function can be overridden by child class

        }

        void print(){
            cout << "Element of x: " << x << " y: " << y << "\n";
        }
};


// STATES

class Liquid: public Element{

};


class Solid: public Element{
    public:
        Solid(int x, int y): Element(x, y){

        }
};


class Gas: public Element{

};


// Intermediate

class MovableSolid: public Solid{
    public:
        MovableSolid(int x, int y): Solid(x, y){

        }

        void update(double deltaTime){
            y++;
        }
};


class ImmovableSolid: public Solid{
    public:
        ImmovableSolid(int x, int y): Solid(x, y){

        }
};


// Types

class Water: public Liquid{

};


class Sand: public MovableSolid{
    public:
        Sand(int x, int y): MovableSolid(x, y){
            colour = {255, 165, 0};
        }
};


class Stone: public ImmovableSolid{
    public:
        Stone(int x, int y): ImmovableSolid(x, y){
            colour = {96, 93, 90};
        }
};


class Smoke: public Gas{

};
