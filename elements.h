#include <SDL2/SDL.h>

// BASE

class Element{
    public:
        int x;
        int y;
        SDL_Colour colour;
        void draw(SDL_Renderer*);
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
