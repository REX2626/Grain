#include <SDL2/SDL.h>

using namespace std;

// BASE

#define AIR_THERM_CONDUCTIVITY 0.05

class Element{
    public:
        int x;
        int y;
        SDL_Colour colour = {106, 13, 173}; // Purple (this should be overridden by child class)
        string tag = ""; // Used to differentiate elements
        bool updated = false;
        bool onFire = false;
        float fireSmokiness = 0.02; // higher is more smoky
        float temperature = 0; // measured in °C compared to air temperature
        float thermalConductivity = 0.05; // 0 to 1
        float igniteTemp = 100;

        Element(int x, int y): x(x), y(y){
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
                } else {
                    heat_col = {255, 255, 255, 200};
                }
                SDL_SetRenderDrawColor(renderer, heat_col.r, heat_col.g, heat_col.b, heat_col.a);
            }


            SDL_Rect rect = {x * size, y * size, size, size};

            SDL_RenderFillRect(renderer, &rect);
        }

        virtual void update(double deltaTime){ // virtual means this function can be overridden by child class

        }

        virtual void setFreeFalling(){}

        virtual void ignite(){}

        virtual bool attemptSetOnFire(){ return false; }

        virtual bool canBeSetOnFire(){ return false; }

        void print(){
            cout << "Element of x: " << x << " y: " << y << "\n";
        }
};
