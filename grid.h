#include <iostream>
using namespace std;

class Grid{
    private:
        Element* matrix[GRID_WIDTH][GRID_HEIGHT]; // Array of pointers, pointing to Elements

    public:

        void reset(){
            for (int x = 0; x < GRID_WIDTH; x++){
                for (int y = 0; y < GRID_HEIGHT; y++){
                    reset(x, y);
                }
            }
        }

        void reset(int x, int y){
            delete matrix[x][y];
            matrix[x][y] = nullptr;
        }

        void resetPtr(){
            for (int x = 0; x < GRID_WIDTH; x++){
                for (int y = 0; y < GRID_HEIGHT; y++){
                    resetPtr(x, y);
                }
            }
        }

        void resetPtr(int x, int y){
            matrix[x][y] = nullptr;
        }

        void set(int x, int y, Element *elementPtr){
            delete matrix[x][y];
            matrix[x][y] = elementPtr;
        }

        Element get(int x, int y){
            return *matrix[x][y];
        }

        Element* getPtr(int x, int y){
            return matrix[x][y];
        }

        // element should not be at the same position as (x, y)
        void move(Element *elementPtr, int x, int y){
            set(x, y, elementPtr);
            resetPtr(elementPtr->x, elementPtr->y); // delete pointer of current position
            matrix[x][y]->setX(x);
            matrix[x][y]->setY(y);
        }

        void moveTo(Element *element, int x, int y){
            int xPos = element->x;
            int yPos = element->y;
            int startX = xPos;
            int startY = yPos;

            int yDirection = 1;
            if (y < yPos){yDirection = -1;}

            if (x == xPos){ // Moving vertically
                for (int i = 0; i < abs(y - startY); i++){
                    if (!inBounds(xPos, yPos + yDirection) || (isFull(xPos, yPos + yDirection) && get(xPos, yPos + yDirection).state != "liquid")){break;}
                    yPos += yDirection;
                }

            }
            else { // Not moving vertically
                float gradient = (float)(y - yPos) / (float)(x - xPos);

                if (abs(gradient) > 1){
                    gradient = 1 / gradient;

                    for (int i = 1; i <= abs(y - startY); i++){
                        int newX = startX + round(i * gradient);
                        if (!inBounds(newX, yPos + yDirection) || (isFull(newX, yPos + yDirection) && get(newX, yPos + yDirection).state != "liquid")){break;}
                        xPos = newX;
                        yPos += yDirection;
                    }
                }
                else {
                    int xDirection = 1;
                    if (x < xPos){xDirection = -1;}
                    for (int i = 1; i <= abs(x - startX); i++){
                        int newY = startY + round(i * gradient);
                        if (!inBounds(xPos + xDirection, newY) || (isFull(xPos + xDirection, newY) && get(xPos + xDirection, newY).state != "liquid")){break;}
                        xPos += xDirection;
                        yPos = newY;
                    }
                }
            }

            if (xPos != element->x || yPos != element->y){ // If the move positions have changed, move there
                if (isFull(xPos, yPos)){
                    swap(element, xPos, yPos);
                }else{
                    move(element, xPos, yPos);
                }
            }
        }

        void swap(Element *element, int x, int y){
            Element* swappedElement = getPtr(x, y);
            matrix[x][y] = matrix[element->x][element->y];
            matrix[element->x][element->y] = swappedElement;

            matrix[element->x][element->y]->setX(element->x);
            matrix[element->x][element->y]->setY(element->y);

            matrix[x][y]->setX(x);
            matrix[x][y]->setY(y);
        }

        bool isFull(int x, int y){
            return (bool) matrix[x][y];
        }

        bool isEmpty(int x, int y){
            return !isFull(x, y);
        }

        bool inBounds(int x, int y){
            return x >= 0 && y >=0 && x < GRID_WIDTH && y < GRID_HEIGHT;
        }
};
