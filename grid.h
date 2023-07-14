class Grid{
    private:
        Element* matrix[GRID_WIDTH][GRID_HEIGHT]; // Array of pointers, pointing to Elements
        Element* matrixCopy[GRID_WIDTH][GRID_HEIGHT]; // Copy to ensure elements can't be updated multiple times

    public:
        void copy(){
            for (int x = 0; x < GRID_WIDTH; x++){
                for (int y = 0; y < GRID_HEIGHT; y++){
                    matrixCopy[x][y] = matrix[x][y];
                }
            }
        }

        void reset(){
            for (int x = 0; x < GRID_WIDTH; x++){
                for (int y = 0; y < GRID_HEIGHT; y++){
                    matrix[x][y] = nullptr;
                }
            }
            copy();
        }

        Element getCopy(int x, int y){
            return *matrixCopy[x][y];
        }

        Element* getPtrCopy(int x, int y){
            return matrixCopy[x][y];
        }

        bool isFullCopy(int x, int y){
            return (bool) matrixCopy[x][y];
        }

        void set(int x, int y, Element* elementPtr){
            matrix[x][y] = elementPtr;
        }

        Element get(int x, int y){
            return *matrix[x][y];
        }

        Element* getPtr(int x, int y){
            return matrix[x][y];
        }

        void move(Element element, int x, int y){
            matrix[x][y] = matrix[element.x][element.y];
            matrix[element.x][element.y] = nullptr;
            matrix[x][y]->setX(x);
            matrix[x][y]->setY(y);
        }

        void moveTo(Element element, int x, int y){
            int xPos = element.x;
            int yPos = element.y;

            if (x == xPos){ // Moving vertically
                int yDirection = 1;
                if (y < yPos){yDirection = -1;}

                for (int i = 0; i < abs(y - yPos); i++){
                    if (!inBounds(xPos, yPos + yDirection) || isFull(xPos, yPos + yDirection)){break;}
                    yPos += yDirection;
                }

            }
            else { // Not moving vertically
                int xDirection = 1;
                if (x < xPos){xDirection = -1;}

                float gradient = (float)(y - yPos) / (float)(x - xPos);

                float startY = yPos;
                float dx = abs(x - xPos);

                for (int i = 1; i <= dx; i++){
                    int newY = startY + round(i * gradient);
                    if (!inBounds(xPos + xDirection, newY) || isFull(xPos + xDirection, newY)){break;}
                    xPos += xDirection;
                    yPos = newY;
                }
            }

            if (xPos != element.x || yPos != element.y){ // If the move positions have changed, move there
                move(element, xPos, yPos);
            }
        }

        void swap(Element element, int x, int y){
            Element* swappedElement = getPtr(x, y);
            matrix[x][y] = matrix[element.x][element.y];
            matrix[element.x][element.y] = swappedElement;

            matrix[element.x][element.y]->setX(element.x);
            matrix[element.x][element.y]->setY(element.y);

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
