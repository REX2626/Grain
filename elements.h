#include <SDL2/SDL.h>

using namespace std;

void updateTemp(Element *e) {
    int x = e->x;
    int y = e->y;
    int dx, dy;
    float weightedTempDiff = 0.0;
    float neighTemp;
    for (dx=-1; dx<=1; dx+=2) // loop thru 4 neighbours
    for (dy=-1; dy<=1; dy+=2) {
        if (!grid.inBounds(x+dx, y+dy)) {continue;}
        if (grid.isEmpty(x+dx, y+dy)) {
            weightedTempDiff += - AIR_THERM_CONDUCTIVITY * e->temperature;
            continue;
        }
        neighTemp = grid.getPtr(x+dx, y+dy)->temperature;
        weightedTempDiff += grid.getPtr(x+dx, y+dy)->thermalConductivity * (neighTemp - e->temperature);
    }
    e->temperature += weightedTempDiff/6; // div by 6 to ensure cant have temp overflow
}

// STATES

class Liquid: public Element{
    public:
        int dispersion; // greater -> moves further each frame
        double density; // greater -> will sink below other liquids with a lower density

        Liquid(int x, int y): Element(x, y){
            state = "liquid";
        }

        double getDensity(){return density;}

        void findNewPosition(int *xPtr, int *yPtr){
            // if cell below is free, then move there
            if (grid.inBounds(x, y + 1) && grid.isEmpty(x, y + 1)){
                *xPtr = x;
                *yPtr = y + 1;
                return;
            }

            // find closest downhill
            int pX;
            int pY;
            int dir = 2*(rand()%2) - 1; // either -1 or +1
            int dist1 = 0;
            int dist2 = 0;
            int pX1 = -1;
            int pX2 = -1;

            for (int step = 1; step < dispersion+1; step++){
                // attempt to find downward cell
                if (!grid.inBounds(x + dir*step, y) || grid.isFull(x + dir*step, y)) {break;}
                dist1++;
                if (grid.inBounds(x + dir*step, y + 1) && grid.isEmpty(x + dir*step, y + 1)) {
                    pX1 = x + dir*step;
                    break;
                }
            }

            for (int step = 1; step < dispersion+1; step++){
                // attempt to find downward cell
                if (!grid.inBounds(x - dir*step, y) || grid.isFull(x - dir*step, y)) {break;}
                dist2++;
                if (grid.inBounds(x - dir*step, y + 1) && grid.isEmpty(x - dir*step, y + 1)) {
                    pX2 = x - dir*step;
                    break;
                }
            }

            if (pX1 == -1 && pX2 == -1) { // can't find downward cell
                // find furthest space to the side
                if (dist1 >= dist2) {pX = x + dir*dist1;}
                else {pX = x - dir*dist2;}
                pY = y;
            }
            else if (pX2 == -1 || (pX1 != -1 && dist1 <= dist2)){ // dist1 is shorter or equal to dist 2
                pX = pX1;
                pY = y + 1;
            }
            else if (pX1 == -1 || (pX2 != -1 && dist2 < dist1)){ // dist2 is shorter
                pX = pX2;
                pY = y + 1;
            }

            *xPtr = pX;
            *yPtr = pY;
        }

        void update(double deltaTime){
            // Find position moving through air, seeks out closest downhill position
            int pX;
            int pY;
            findNewPosition(&pX, &pY);
            if (pX != x || pY != y) {grid.move(this, pX, pY);}

            // Displacing lower density liquid
            else if (grid.inBounds(x, y+1) && grid.isFull(x, y+1) && grid.get(x, y+1).state == "liquid" &&
                     grid.getPtr(x, y+1)->getDensity() < density && (density - grid.getPtr(x, y+1)->getDensity()) > (float)rand()/RAND_MAX){ // bigger density difference, greater chance
                        grid.swap(this, x, y+1);
            }

            // Moves sideways randomly through a lower density liquid
            else if (y+1 == GRID_HEIGHT || grid.isFull(x, y+1)){
                int dir = 2*(rand()%2) - 1; // either -1 or +1

                // through lower density liquid
                if (grid.inBounds(x + dir, y) && grid.getPtr(x + dir, y)->state == "liquid" && grid.getPtr(x + dir, y)->getDensity() < density){
                    grid.swap(this, x + dir, y);
                }
                else if (grid.inBounds(x - dir, y) && grid.getPtr(x - dir, y)->state == "liquid" && grid.getPtr(x - dir, y)->getDensity() < density){
                    grid.swap(this, x - dir, y);
                }
            }

            updateTemp(this);
        }
};


class Solid: public Element{
    public:
        int fireTicks = 50; // child should override
        SDL_Colour baseColour;
        float fireResistance = 0.95; // higher is more resistant
        float fireDiffusivity = 0.10; // higher is more spread-y

        int fireSpreadRange = 2;

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
            temperature = max((float)150.0, temperature);
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
            if (temperature > igniteTemp) {
                ignite();
            }

            if (!onFire) {return;}

            if (temperature < igniteTemp - 1) {
                putOutFire();
                return;
            }

            // increase temperature
            temperature += 1;

            // reduce fire ticks destroy self if so
            fireTicks --;
            if (fireTicks <= 0) {
                grid.set(x, y, nullptr);
                return;
            }

            // look like fire
            colour.r = (Uint8)(200 + rand()%40);
            colour.g = (Uint8)(rand()%255);
            colour.b = (Uint8)(rand()%10);

            // // spread to neighbours by chance
            // if ((float)rand()/RAND_MAX > fireDiffusivity) {return;}
            // // pick a random neighbour
            // int dx = rand()%(fireSpreadRange*2+1) - fireSpreadRange; // has the effect of being in the range [-fireSpreadRange, fireSpreadRange] (incl. 0)
            // int dy = rand()%(fireSpreadRange*2+1) - fireSpreadRange;
            // if (dx == 0 && dy == 0) {return;}
            // if (grid.isEmpty(x+dx, y+dy)) {return;}
            // if (!grid.getPtr(x+dx, y+dy)->canBeSetOnFire()) {return;}
            // // grid.getPtr(x+dx, y+dy)->attemptSetOnFire();
        }
};


class Gas: public Element{
    public:
        int velX = 0;
        int velY = 0;
        int density = 6; // higher = rises up faster (so the opposite of density then)
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

            updateTemp(this);
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
            movableUpdate(deltaTime);
            updateTemp(this);
            updateFire();
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
                // Displaces liquid
                else if (grid.get(x, y+1).state == "liquid"){
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
            updateTemp(this);
            updateFire();
        }
};


// Types

class Water: public Liquid{
    public:
        Water(int x, int y): Liquid(x, y){
            colour = {(Uint8)(170 + rand() % 20), (Uint8)(210 + rand() % 20), (Uint8)(230 + rand() % 20)};
            tag = "water";
            dispersion = 5;
            density = 0.1;
        }
};

class Oil: public Liquid{
    public:
        Oil(int x, int y): Liquid(x, y){
            int random = rand() % 4;
            colour = {(Uint8)(10 + random), (Uint8)(10 + random), (Uint8)(10 + random)};
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
            fireResistance = 0.997;
            fireDiffusivity = 0.01;
            fireTicks = 20 + rand() % 10;
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
            fireResistance = 0.990;
            fireDiffusivity = 0.02;
            fireTicks = 50 + rand() % 10;
            initSolid();
        }
};

class Coal: public MovableSolid{
    public:
        Coal(int x, int y): MovableSolid(x, y){
            int random = rand() % 15;
            baseColour = {(Uint8)(25 + random), (Uint8)(29 + random), (Uint8)(27 + random)};
            tag = "coal";
            inertialResistance = 0.8;
            friction = 0.8;
            fireResistance = 0.40;
            fireDiffusivity = 0.10;
            fireTicks = 500 + rand() % 100;
            initSolid();
        }
};


class Stone: public ImmovableSolid{
    public:
        Stone(int x, int y): ImmovableSolid(x, y){
            int random = rand() % 20;
            baseColour = {(Uint8)(96 + random), (Uint8)(93 + random), (Uint8)(90 + random)};
            tag = "stone";
            fireResistance = 1.00;
            fireDiffusivity = 0.02;
            fireTicks = 100 + rand() % 50;
            initSolid();
        }
};

class Wood: public ImmovableSolid {
    public:
    Wood(int x, int y): ImmovableSolid(x, y) {
        int random = rand()%20 - 10;
        baseColour = {(Uint8)(100+random), (Uint8)(75+random), (Uint8)(50+random)};
        tag = "wood";
        fireResistance = 0.30;
        fireDiffusivity = 0.20;
        fireTicks = 150 + rand()%50;
        fireSpreadRange = 3;
        initSolid();
    }
};


class Smoke: public Gas{
    public:
        Smoke(int x, int y): Gas(x, y){
            int random = rand() % 20;
            baseColour = {(Uint8)(200 + random), (Uint8)(200 + random), (Uint8)(200 + random)};
            tag = "smoke";
            density = 6;
            temperature = 100; // https://www.google.com/search?q=temperature+of+smoke+degrees+c
        }
};
