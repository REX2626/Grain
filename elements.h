#include <SDL2/SDL.h>

using namespace std;

// STATES

class Liquid: public Element{
    public:
        int dispersion; // greater -> moves further each frame
        double density; // greater -> will sink below other liquids with a lower density

        Liquid(int x, int y): Element(x, y){
            state = "liquid";
        }

        double getDensity(){return density;}

        void update(double deltaTime){
            // Falling
            if (grid.inBounds(x, y+1) && grid.isEmpty(x, y+1)){
                grid.move(this, x, y+1);
            }
            // Displacing lower density liquid
            else if (grid.inBounds(x, y+1) && grid.isFull(x, y+1) && grid.get(x, y+1).state == "liquid" &&
                     grid.getPtr(x, y+1)->getDensity() < density && (density - grid.getPtr(x, y+1)->getDensity()) < (float)rand()/RAND_MAX){ // bigger density difference, greater chance
                        grid.swap(this, x, y+1);
            }
            // Moves sideways randomly, either through air or a lower density liquid
            else if (y+1 == GRID_HEIGHT || grid.isFull(x, y+1)){
                int dir = 2*(rand()%2) - 1; // either -1 or +1

                // through air
                if (grid.inBounds(x + dir, y) && grid.isEmpty(x + dir, y)){
                    grid.moveTo(this, x + dir*dispersion, y);
                }
                else if (grid.inBounds(x - dir, y) && grid.isEmpty(x - dir, y)){
                    grid.moveTo(this, x - dir*dispersion, y);
                }
                // through lower density liquid
                else if (grid.inBounds(x + dir, y) && grid.getPtr(x + dir, y)->state == "liquid" && grid.getPtr(x + dir, y)->getDensity() < density){
                    grid.swap(this, x + dir, y);
                }
                else if (grid.inBounds(x - dir, y) && grid.getPtr(x + dir, y)->state == "liquid" && grid.getPtr(x - dir, y)->getDensity() < density){
                    grid.swap(this, x - dir, y);
                }
            }
        }
};


class Solid: public Element{
    public:
        bool onFire;
        int fireTicks; // child should override
        SDL_Colour baseColour;
        float fireResistance;

        Solid(int x, int y): Element(x, y){
            state = "solid";
        }

        void initSolid() {
            onFire = false;
            colour = baseColour;
        }

        bool canBeSetOnFire() {return true;}

        void ignite() {
            if (onFire) {return;}
            onFire = true;
        }

        bool attemptSetOnFire() {
            if ((float)rand()/RAND_MAX <= fireResistance) {return false;}
            ignite();
            return true;
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
        int density = 6; // higher = rises up faster
        SDL_Colour baseColour;

        Gas(int x, int y): Element(x, y){
            state = "gas";
        }

        void update(double deltaTime){
            velX = rand() % 11 - 5;
            velY = rand() % 10 - density;
            grid.moveTo(this, x + velX, y + velY);

            // Darkens based on how many gas particles are around it
            int neighbours = 0;
            for (int i = -2; i < 3; i++){
                for (int j = -2; j < 3; j++){
                    if (i == 0 && j == 0) {break;}
                    if (!grid.inBounds(x + i, y + j)){
                        neighbours++;
                    }
                    else if (grid.isFull(x + i, y + j) && grid.get(x + i, y + j).tag == "smoke"){
                        neighbours++;
                    }
                }
            }

            colour.r = baseColour.r - 5 * neighbours;
            colour.g = baseColour.g - 5 * neighbours;
            colour.b = baseColour.b - 5 * neighbours;
        }
};


// Intermediate

class MovableSolid: public Solid{
    public:
        bool freeFalling = true;
        double inertialResistance; // between 0 and 1
        double friction; // between 0 and 1
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
            velX -= velX * friction; // decrease velX proportional to friction

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
            density = 0.1;
        }
};

class Oil: public Liquid{
    public:
        Oil(int x, int y): Liquid(x, y){
            int random = rand() % 5;
            colour = {(Uint8)(15 + random), (Uint8)(15 + random), (Uint8)(15 + random)};
            tag = "oil";
            dispersion = 2;
            density = 0.5;
        }
};

class Slime: public Liquid{
    public:
        Slime(int x, int y): Liquid(x, y){
            colour = {(Uint8)(20 + rand() % 20), (Uint8)(200 + rand() % 20), (Uint8)(100 + rand() % 20)};
            tag = "slime";
            dispersion = 1;
            density = 0.8;
        }
};


class Sand: public MovableSolid{
    public:
        Sand(int x, int y): MovableSolid(x, y){
            baseColour = {(Uint8)(225 + rand() % 20), (Uint8)(160 + rand() % 20), 0};
            tag = "sand";
            inertialResistance = 0.1;
            friction = 0.2;
            initSolid();
        }
};

class Dirt: public MovableSolid{
    public:
        Dirt(int x, int y): MovableSolid(x, y){
            baseColour = {(Uint8)(100 + rand() % 20), (Uint8)(50 + rand() % 10), 0};
            tag = "dirt";
            inertialResistance = 0.6;
            friction = 0.6;
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
            friction = 0.8;
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
            baseColour = {(Uint8)(200 + random), (Uint8)(200 + random), (Uint8)(200 + random)};
            tag = "smoke";
        }
};
