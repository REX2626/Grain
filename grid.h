class Grid{
    private:
        Element* matrix[GRID_WIDTH][GRID_HEIGHT]; // Array of pointers, pointing to Elements

    public:
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

        bool isFull(int x, int y){
            return (bool) matrix[x][y];
        }

        bool isEmpty(int x, int y){
            return !isFull(x, y);
        }
};
