#include <SDL2/SDL.h>

using namespace std;

void updateTemp(Element *e) {
    int x = e->x;
    int y = e->y;
    int dx, dy;
    float weightedTempDiff = 0.0;
    float neighTemp, cornerFac;
    for (dx=-1; dx<=1; dx++) // loop thru 8 neighbours
    for (dy=-1; dy<=1; dy++) {
        if (dx == 0 && dy == 0) {continue;}
        if (!grid.inBounds(x+dx, y+dy)) {continue;}
        cornerFac = 1;
        if (abs(dx) == 1 && abs(dy) == 1) {
            cornerFac = 0.707;
        }
        if (grid.isEmpty(x+dx, y+dy)) {
            weightedTempDiff += - cornerFac * AIR_THERM_CONDUCTIVITY * e->temperature;
            continue;
        }
        neighTemp = grid.getPtr(x+dx, y+dy)->temperature;
        weightedTempDiff += cornerFac * grid.getPtr(x+dx, y+dy)->thermalConductivity * (neighTemp - e->temperature);
    }
    weightedTempDiff *= 80 + rand()%40;
    e->temperature += weightedTempDiff/e->heatCapacity;
}

// STATES

class Liquid: public Element{
    public:
        int dispersion; // greater -> checks further to the side for downward cell each frame
        int density; // measured in kg/m^3, greater -> will sink below other liquids with a lower density

        Liquid(int x, int y): Element(x, y){
            initLiquid();
        }

        void initLiquid() {
            state = "liquid";
            initElement();
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
                // attempt to find downward cell, can pass through same liquid
                if (!grid.inBounds(x + dir*step, y) || (grid.isFull(x + dir*step, y) && grid.getPtr(x + dir*step, y)->tag != tag)) {break;}
                dist1++;
                if (grid.inBounds(x + dir*step, y + 1) && grid.isEmpty(x + dir*step, y + 1)) {
                    pX1 = x + dir*step;
                    break;
                }
            }

            for (int step = 1; step < dispersion+1; step++){
                // attempt to find downward cell, can pass through same liquid
                if (!grid.inBounds(x - dir*step, y) || (grid.isFull(x - dir*step, y) && grid.getPtr(x - dir*step, y)->tag != tag)) {break;}
                dist2++;
                if (grid.inBounds(x - dir*step, y + 1) && grid.isEmpty(x - dir*step, y + 1)) {
                    pX2 = x - dir*step;
                    break;
                }
            }

            if (pX1 == -1 && pX2 == -1) { // can't find downward cell
                // find furthest space to the side
                dist1 = 0;
                dist2 = 0;
                for (int step = 1; step < dispersion+1; step++) {
                    if (!grid.inBounds(x + dir*step, y) || grid.isFull(x + dir*step, y)) {break;}
                    dist1++;
                }
                for (int step = 1; step < dispersion+1; step++) {
                    if (!grid.inBounds(x - dir*step, y) || grid.isFull(x - dir*step, y)) {break;}
                    dist2++;
                }
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

        void update(double deltaTime) {
            updateLiquid(deltaTime);
            updateTemp(this);
            updateFire();
        }

        void updateLiquid(double deltaTime){
            // Find position moving through air, seeks out closest downhill position
            int pX;
            int pY;
            findNewPosition(&pX, &pY);

            // Displacing lower density liquid
            if (grid.inBounds(x, y+1) && grid.isFull(x, y+1) && grid.get(x, y+1).state == "liquid" &&
                     grid.getPtr(x, y+1)->getDensity() < density && grid.getPtr(x, y+1)->getDensity()/density < (float)rand()/RAND_MAX){ // bigger density difference, greater chance
                        grid.swap(this, x, y+1);
            }

            else if (pX != x || pY != y) {grid.move(this, pX, pY);}

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
        }
};


class Solid: public Element{
    public:
        Solid(int x, int y): Element(x, y){
            initSolid();
        }

        void initSolid() {
            state = "solid";
            initElement();
        }

        bool canBeSetOnFire() {return true;}
};


class Gas: public Element{
    public:
        int velX = 0;
        int velY = 0;
        int buoyancy = 6; // higher -> rises up faster (opposite of density)
        int lifetime = 1000; // measured in frames
        int timeAlive = 0;

        Gas(int x, int y): Element(x, y){
            initGas();
        }

        void initGas() {
            state = "gas";
            initElement();
        }

        void update(double deltaTime) {
            updateGas();
            updateTemp(this);
            updateFire();
        }

        void updateGas(){
            timeAlive++;
            if (timeAlive > lifetime) {health = 0; return;}

            velX = rand() % 11 - 5; // -5 to +5
            velY = rand() % 10 - buoyancy;
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
                // Displaces liquid or gas
                else if (grid.get(x, y+1).state == "liquid" || grid.get(x, y+1).state == "gas"){
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
            baseColour = {(Uint8)(170 + rand() % 20), (Uint8)(210 + rand() % 20), (Uint8)(230 + rand() % 20)};
            tag = "water";
            dispersion = 5;
            density = 997;
            heatCapacity = 4186;
            thermalConductivity = 0.65;
            initLiquid();
        }
};

class Oil: public Liquid{
    public:
        Oil(int x, int y): Liquid(x, y){
            int random = rand() % 4;
            baseColour = {(Uint8)(10 + random), (Uint8)(10 + random), (Uint8)(10 + random)};
            tag = "oil";
            dispersion = 2;
            density = 900;
            heatCapacity = 800;
            thermalConductivity = 0.7;
            igniteTemp = 100;
            initLiquid();
        }

        bool canBeSetOnFire() {
            return true;
        }
};

class Slime: public Liquid{
    public:
        Slime(int x, int y): Liquid(x, y){
            baseColour = {(Uint8)(20 + rand() % 20), (Uint8)(200 + rand() % 20), (Uint8)(100 + rand() % 20)};
            tag = "slime";
            dispersion = 1;
            density = 2190;
            heatCapacity = 3000; // a bit less than water ig
            thermalConductivity = 0.5;
            acidResistance = 10;
            initLiquid();
        }
};


class Sand: public MovableSolid{
    public:
        Sand(int x, int y): MovableSolid(x, y){
            baseColour = {(Uint8)(225 + rand() % 20), (Uint8)(160 + rand() % 20), 0};
            tag = "sand";
            inertialResistance = 0.1;
            friction = 0.2;
            heatCapacity = 830;
            thermalConductivity = 0.15;
            initSolid();
        }

        bool canBeSetOnFire() {return false;}
};

class Dirt: public MovableSolid{
    public:
        Dirt(int x, int y): MovableSolid(x, y){
            baseColour = {(Uint8)(100 + rand() % 20), (Uint8)(50 + rand() % 10), 0};
            tag = "dirt";
            inertialResistance = 0.6;
            friction = 0.6;
            heatCapacity = 800;
            thermalConductivity = 0.36;
            igniteTemp = 600;
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
            heatCapacity = 1380; // (bituminous)
            thermalConductivity = 0.17;
            igniteTemp = 300;
            initSolid();
        }
};


class Stone: public ImmovableSolid{
    public:
        Stone(int x, int y): ImmovableSolid(x, y){
            int random = rand() % 20;
            baseColour = {(Uint8)(96 + random), (Uint8)(93 + random), (Uint8)(90 + random)};
            tag = "stone";
            heatCapacity = 1000;
            thermalConductivity = 0.9;
            igniteTemp = 800;
            acidResistance = 2;
            initSolid();
        }
};

class Wood: public ImmovableSolid {
    public:
    Wood(int x, int y): ImmovableSolid(x, y) {
        int random = rand()%20 - 10;
        baseColour = {(Uint8)(100+random), (Uint8)(75+random), (Uint8)(50+random)};
        tag = "wood";
        heatCapacity = 1300;
        thermalConductivity = 0.12;
        igniteTemp = 200;
        initSolid();
    }
};


class Smoke: public Gas{
    public:
        Smoke(int x, int y): Gas(x, y){
            int random = rand() % 20;
            baseColour = {(Uint8)(200 + random), (Uint8)(200 + random), (Uint8)(200 + random)};
            tag = "smoke";
            buoyancy = 6;
            lifetime = 600 + rand()%801; // 600-1400
            temperature = 100; // https://www.google.com/search?q=temperature+of+smoke+degrees+c
            heatCapacity = 1000;
            thermalConductivity = 0.05;
            initGas();
        }

        bool canBeSetOnFire() {return false;}
};


class Acid: public Liquid{
    public:
        int acidStrength;

        Acid(int x, int y): Liquid(x, y){
            baseColour = {(Uint8)(130 + rand() % 40), (Uint8)(40 + rand() % 20), (Uint8)(20 + rand() % 20)};
            tag = "acid";
            dispersion = 2;
            density = 1200;
            heatCapacity = 2000;
            thermalConductivity = 0.4;
            acidStrength = 5;
            initLiquid();
        }

        int acidify() {return 0;}

        void update(double deltaTime) {
            int xDir = rand()%2 * 2 - 1; // -1 or +1
            int yDir = rand()%2 * 2 - 1; // -1 or +1

            colour = {(Uint8)(130 + rand() % 40), (Uint8)(40 + rand() % 20), (Uint8)(20 + rand() % 20)}; // random colour

            // check for 8 adjacent neighbours, and try to acidify
            for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++) {

                if (i == 0 && j == 0) {continue;}
                if (!grid.inBounds(x + i*xDir, y + j*yDir) || grid.isEmpty(x + i*xDir, y + j*yDir)) {continue;}
                string tag = grid.getPtr(x + i*xDir, y + j*yDir)->tag;
                if (tag == "acid" || tag == "smoke") {continue;}

                health -= 100.0/acidStrength * grid.getPtr(x + i*xDir, y + j*yDir)->acidify();

                if (rand()%4 == 0 && grid.inBounds(x, y - 1) && grid.isEmpty(x, y - 1)) { // 25% chance to generate smoke
                    Smoke *smoke = new Smoke(x, y - 1);
                    smoke->lifetime = 80 + rand()%41; // 80-120 ticks
                    grid.set(x, y - 1, smoke);
                }

                Liquid::update(deltaTime);
                return;
            }

            Liquid::update(deltaTime);
        }
};
