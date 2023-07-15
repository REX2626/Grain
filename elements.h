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
                grid.move(this, x, y+1);
            }
            // Moves sideways randomly
            else if (y+1 == GRID_HEIGHT || grid.isFull(x, y+1)){
                int dir = 2*(rand()%2) - 1; // either -1 or +1

                if (grid.inBounds(x + dir, y) && grid.isEmpty(x + dir, y)){
                    grid.moveTo(this, x + dir*dispersion, y);
                }
                else if (grid.inBounds(x - dir, y) && grid.isEmpty(x - dir, y)){
                    grid.moveTo(this, x - dir*dispersion, y);
                }
            }
        }
};


class Solid: public Element{
    public:
        bool onFire;
        int fireTicks; // child should override
        SDL_Colour baseColour;

        Solid(int x, int y): Element(x, y){
        }

        void initSolid() {
            onFire = false;
            colour = baseColour;
            canBeSetOnFire = true;
        }

        void setOnFire() {
            if (onFire) {return;}
            onFire = true;
        }

        void putOutFire() {
            if (!onFire) {return;}
            onFire = false;
            colour = baseColour;
        }

        void updateFire() {
            if (onFire) {
                colour.r = (Uint8)(200 + rand()%40);
                colour.g = (Uint8)(rand()%255);
                colour.b = (Uint8)(rand()%10);
            }
        }
};


class Gas: public Element{
    public:
        int velX = 0;
        int velY = 0;

        Gas(int x, int y): Element(x, y){

        }

        void update(double deltaTime){
            velX = rand() % 11 - 5;
            velY = rand() % 10 - 8;
            grid.moveTo(this, x + velX, y + velY);
        }
};


// Intermediate

class MovableSolid: public Solid{
    public:
        bool freeFalling = true;
        double inertialResistance; // between 0 and 1
        double friction;
        int distFallen = 0;
        int prevX;
        int prevY;
        int velX = 0;

        MovableSolid(int x, int y): Solid(x, y), prevX(x), prevY(y){
            initSolid();
        }

        void update(double deltaTime) {
            updateFire();
            movableUpdate(deltaTime);
        }

        void movableUpdate(double deltaTime){
            prevX = x;
            prevY = y;
            velX *= max(0.0, 1 - deltaTime*friction); // decrease velX proportional to friction

            // Moving sideways
            if (velX > 0){
                // Move right if cell is empty
                if (grid.inBounds(x+1, y) && grid.isEmpty(x+1, y)){
                    grid.move(this, x+1, y);
                }
                else {
                    velX = 0;
                }
            }
            else if (velX < 0){
                // Move left if cell is empty
                if (grid.inBounds(x-1, y) && grid.isEmpty(x-1, y)){
                    grid.move(this, x-1, y);
                }
                else {
                    velX = 0;
                }
            }

            // Only move down if cell below is in bounds
            if (grid.inBounds(x, y+1)){
                // Falling
                if (grid.isEmpty(x, y+1)){
                    grid.move(this, x, y+1);
                }
                // Displaces water
                else if (grid.get(x, y+1).tag == "water"){
                    grid.swap(this, x, y+1);
                }
                // Moves diagonally randomly
                else if (freeFalling && grid.isFull(x, y+1)){
                    int dir = 2*(rand()%2)-1; // -1 or +1
                    if (grid.inBounds(x + dir, y+1) && grid.isEmpty(x + dir, y+1)){
                        grid.move(this, x + dir, y+1);
                    }
                    else if (grid.inBounds(x - dir, y+1) && grid.isEmpty(x - dir, y+1)){
                        grid.move(this, x - dir, y+1);
                    }
                }
            }

            // If moved, freeFalling is true
            if (prevY == y){
                if (freeFalling){
                    freeFalling = false;
                    if (velX > 0){velX += distFallen;}
                    else if (velX < 0){velX -= distFallen;}
                    else {velX = (rand()%2 * 2 - 1) * max(5, distFallen);}
                    velX *= 4 * rand()/RAND_MAX; // arbitrary
                    distFallen = 0;
                }
            }
            else {
                freeFalling = true;
                distFallen += y - prevY;
                setAdjacentFreeFalling();
            }
        }

        void setAdjacentFreeFalling(){
            // Call setFreeFalling on the left and right cells
            if (grid.inBounds(x-1, y) && grid.isFull(x-1, y)) {grid.getPtr(x-1, y)->setFreeFalling();}
            if (grid.inBounds(x+1, y) && grid.isFull(x+1, y)) {grid.getPtr(x+1, y)->setFreeFalling();}
            if (grid.inBounds(x-1, y-1) && grid.isFull(x-1, y-1)) {grid.getPtr(x-1, y-1)->setFreeFalling();}
            if (grid.inBounds(x+1, y-1) && grid.isFull(x+1, y-1)) {grid.getPtr(x+1, y-1)->setFreeFalling();}
        }

        void setFreeFalling(){
            if (inertialResistance < (float) rand()/RAND_MAX){
                freeFalling = true;
            }
        }
};


class ImmovableSolid: public Solid{
    public:
        ImmovableSolid(int x, int y): Solid(x, y){
            initSolid();
        }

        void update(double deltaTime) {
            updateFire();
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
            baseColour = {(Uint8)(225 + rand() % 20), (Uint8)(160 + rand() % 20), 0};
            tag = "sand";
            inertialResistance = 0.1;
            friction = 40;
            initSolid();
        }
};

class Dirt: public MovableSolid{
    public:
        Dirt(int x, int y): MovableSolid(x, y){
            baseColour = {(Uint8)(100 + rand() % 20), (Uint8)(50 + rand() % 10), 0};
            tag = "dirt";
            inertialResistance = 0.9;
            friction = 2000;
            initSolid();
        }
};

class Coal: public MovableSolid{
    public:
        Coal(int x, int y): MovableSolid(x, y){
            int random = rand() % 15;
            baseColour = {(Uint8)(30 + random), (Uint8)(34 + random), (Uint8)(32 + random)};
            tag = "coal";
            inertialResistance = 0.8;
            friction = 1000;
            initSolid();
        }
};


class Stone: public ImmovableSolid{
    public:
        Stone(int x, int y): ImmovableSolid(x, y){
            int random = rand() % 20;
            baseColour = {(Uint8)(96 + random), (Uint8)(93 + random), (Uint8)(90 + random)};
            tag = "stone";
            initSolid();
        }
};


class Smoke: public Gas{
    public:
        Smoke(int x, int y): Gas(x, y){
            int random = rand() % 20;
            colour = {(Uint8)(200 + random), (Uint8)(200 + random), (Uint8)(200 + random)};
            tag = "smoke";
        }
};
