#include <SDL2/SDL.h>

using namespace std;

// BASE

#define AIR_THERM_CONDUCTIVITY 0.024

class Element{
    public:
        int x;
        int y;
        SDL_Colour colour = {106, 13, 173}; // Purple (this should be overridden by child class)
        SDL_Colour baseColour = {106, 13, 173};
        string state = "";
        string tag = ""; // Used to differentiate elements
        bool updated = false;
        bool onFire = false;
        float fireSmokiness = 0.02; // higher is more smoky
        float temperature = 0; // measured in °C compared to air temperature
        float thermalConductivity = 0.05; // 0 to 1
        float igniteTemp = 100;
        int fireTicks = 1000;
        float health = 100;
        float heatCapacity = 1; // must be greater than 1

        Element(int x, int y): x(x), y(y){
            initElement();
        }

        void initElement() {
            colour = baseColour;
            onFire = false;
            thermalConductivity *= 0.6 + 1.07*((float)rand()/RAND_MAX);
            fireTicks *= 0.6 + 1.07*((float)rand()/RAND_MAX);
            heatCapacity *= 0.6 + 1.07*((float)rand()/RAND_MAX);
        }

        void setX(int X){
            x = X;
        }

        void setY(int Y){
            y = Y;
        }

        void render(SDL_Renderer* renderer, int size, bool heatmap){
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
            if (heatmap) {
                SDL_Colour heat_col;
                if (temperature < -100) {
                    heat_col = {50, 50, 50, 200};
                } else if (temperature < -50) {
                    heat_col = {0, 0, 200, 50};
                } else if (temperature < -20) {
                    heat_col = {0, 255, 200, 50};
                } else if (temperature < 20) {
                    heat_col = {0, 255, 0, 50};
                } else if (temperature < 50) {
                    heat_col = {0, 255, 0, 200};
                } else if (temperature < 100) {
                    heat_col = {255, 255, 0, 200};
                } else if (temperature < 150) {
                    heat_col = {255, 165, 0, 200};
                } else if (temperature < 200) {
                    heat_col = {255, 127, 80, 200};
                } else if (temperature < 300) {
                    heat_col = {255, 69, 0, 200};
                } else if (temperature < 500) {
                    heat_col = {255, 0, 0, 200};
                } else {
                    heat_col = {255, 255, 255, 200};
                }
                SDL_SetRenderDrawColor(renderer, heat_col.r, heat_col.g, heat_col.b, heat_col.a);
            }


            SDL_Rect rect = {x * size, y * size, size, size};

            SDL_RenderFillRect(renderer, &rect);
        }

        virtual void update(double deltaTime){} // virtual means this function can be overridden by child class

        void updateFire() {
            if (!canBeSetOnFire()) {return;}

            if (temperature > igniteTemp) {
                ignite();
            }

            if (!onFire) {return;}

            if (temperature < igniteTemp - 1) {
                putOutFire();
                return;
            }

            // increase temperature
            temperature += 5;

            // reduce fire ticks destroy self if so
            fireTicks --;
            if (fireTicks <= 0) {
                health = 0;
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

        virtual void setFreeFalling(){}

        void ignite() {
            if (onFire) {return;}
            temperature = max((float)igniteTemp+1, temperature);
            onFire = true;
        }

        void putOutFire() {
            if (!onFire) {return;}
            onFire = false;
            colour = baseColour;
        }

        virtual bool attemptSetOnFire(){ return false; }

        virtual bool canBeSetOnFire(){ return false; }

        virtual double getDensity(){return NAN;}

        void print(){
            cout << "Element of x: " << x << " y: " << y << "\n";
        }
};
