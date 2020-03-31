#ifndef ASTEROID_H
#define ASTEROID_H

#include "Object.hpp"

#define LARGE_ASTEROID      3
#define MEDIUM_ASTEROID     2
#define SMALL_ASTEROID      1
#define DESTROYED_ASTEROID  0

class Asteroid : public Object
{
    private:
        int size;

        void init();
    
    public:
        Asteroid(int size);
        void update();
        void display();

        int getSize();
};

#endif /* ASTEROID_H */