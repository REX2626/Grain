#include <SDL2/SDL.h>

using namespace std;

// STATES

class Liquid: public Element{
    public:
        Liquid(int x, int y): Element(x, y){

        }

        void update(double deltaTime){
            // Falling
            if (y+1 < GRID_HEIGHT && grid.isEmpty(x, y+1)){
                grid.move(*this, x, y+1);
            }
            // Moves sideways randomly
            if (grid.isFull(x, y+1)){
                if (rand() % 20 < 10){
                    if (x-1 >= 0 && grid.isEmpty(x-1, y)){
                        grid.move(*this, x-1, y);
                    }
                    else if (x+1 <= GRID_WIDTH && grid.isEmpty(x+1, y)){
                        grid.move(*this, x+1, y);
                    }
                }
                else{
                    if (x+1 <= GRID_WIDTH && grid.isEmpty(x+1, y)){
                        grid.move(*this, x+1, y);
                    }
                    else if (x-1 >= 0 && grid.isEmpty(x-1, y)){
                        grid.move(*this, x-1, y);
                    }
                }
            }
        }
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
            // Falling
            if (y+1 < GRID_HEIGHT && grid.isEmpty(x, y+1)){
                grid.move(*this, x, y+1);
            }
            // Displaces water
            else if (y+1 < GRID_HEIGHT && grid.get(x, y+1).tag == "water"){
                grid.swap(*this, x, y+1);
            }
            // Moves diagonally
            else if (grid.isFull(x, y+1)){
                if (x-1 >= 0 && grid.isEmpty(x-1, y+1)){
                    grid.move(*this, x-1, y+1);
                }
                else if (x+1 <= GRID_WIDTH && grid.isEmpty(x+1, y+1)){
                    grid.move(*this, x+1, y+1);
                }
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
    public:
        Water(int x, int y): Liquid(x, y){
            colour = {173, 216, 230};
            tag = "water";
        }
};


class Sand: public MovableSolid{
    public:
        Sand(int x, int y): MovableSolid(x, y){
            colour = {255, 165, 0};
            tag = "sand";
        }
};


class Stone: public ImmovableSolid{
    public:
        Stone(int x, int y): ImmovableSolid(x, y){
            colour = {96, 93, 90};
            tag = "stone";
        }
};


class Smoke: public Gas{

};
