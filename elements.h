#include <SDL2/SDL.h>

using namespace std;

// STATES

class Liquid: public Element{
    public:
        int dispersion;

        Liquid(int x, int y): Element(x, y){

        }

        void update(double deltaTime){
            // Falling
            if (grid.inBounds(x, y+1) && grid.isEmpty(x, y+1)){
                grid.move(*this, x, y+1);
            }
            // Moves sideways randomly
            else if (y+1 == GRID_HEIGHT || grid.isFull(x, y+1)){
                if (rand() % 20 < 10){
                    if (grid.inBounds(x-1, y) && grid.isEmpty(x-1, y)){
                        grid.moveTo(*this, x-dispersion, y);
                    }
                    else if (grid.inBounds(x+1, y) && grid.isEmpty(x+1, y)){
                        grid.moveTo(*this, x+dispersion, y);
                    }
                }
                else{
                    if (grid.inBounds(x+1, y) && grid.isEmpty(x+1, y)){
                        grid.moveTo(*this, x+dispersion, y);
                    }
                    else if (grid.inBounds(x-1, y) && grid.isEmpty(x-1, y)){
                        grid.moveTo(*this, x-dispersion, y);
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
            // Return if at the bottom
            if (!grid.inBounds(x, y+1)){return;}

            // Falling
            if (grid.isEmpty(x, y+1)){
                grid.move(*this, x, y+1);
            }
            // Displaces water
            else if (grid.get(x, y+1).tag == "water"){
                grid.swap(*this, x, y+1);
            }
            // Moves diagonally
            else if (grid.isFull(x, y+1)){
                if (grid.inBounds(x-1, y+1) && grid.isEmpty(x-1, y+1)){
                    grid.move(*this, x-1, y+1);
                }
                else if (grid.inBounds(x+1, y+1) && grid.isEmpty(x+1, y+1)){
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
            colour = {(Uint8)(170 + rand() % 20), (Uint8)(210 + rand() % 20), (Uint8)(230 + rand() % 20)};
            tag = "water";
            dispersion = 3;
        }
};


class Sand: public MovableSolid{
    public:
        Sand(int x, int y): MovableSolid(x, y){
            colour = {(Uint8)(225 + rand() % 20), (Uint8)(160 + rand() % 20), 0};
            tag = "sand";
        }
};


class Stone: public ImmovableSolid{
    public:
        Stone(int x, int y): ImmovableSolid(x, y){
            int random = rand() % 20;
            colour = {(Uint8)(96 + random), (Uint8)(93 + random), (Uint8)(90 + random)};
            tag = "stone";
        }
};


class Smoke: public Gas{

};
