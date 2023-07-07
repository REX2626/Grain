#include <SDL2/SDL.h>

using namespace std;

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
            if (y+1 < GRID_HEIGHT && grid.isEmpty(x, y+1)){
                grid.move(*this, x, y+1);
            }
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
