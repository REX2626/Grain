#include <SDL2/SDL.h>

using namespace std;

// BASE

class Element{
    public:
        int x;
        int y;
        SDL_Colour colour = {106, 13, 173}; // Purple (this should be overridden by child class)
        string tag = ""; // Used to differentiate elements

        Element(int x, int y): x(x), y(y){

        }

        void setX(int X){
            x = X;
        }

        void setY(int Y){
            y = Y;
        }

        void render(SDL_Renderer* renderer, int size){
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

            SDL_Rect rect = {x * size, y * size, size, size};

            SDL_RenderFillRect(renderer, &rect);
        }

        virtual void update(double deltaTime){ // virtual means this function can be overridden by child class

        }

        void print(){
            cout << "Element of x: " << x << " y: " << y << "\n";
        }
};
