#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "Vect2D.hpp"
#include "Rect.hpp"
#include "SpriteComponent.hpp"

class Object
{
    protected:
        Rect hitbox;
        int orientation;    // 0 - 360
        int speed;          // 0 - 100
        int acceleration;   // 0 (no acc), 1 (acceleration)

        SpriteComponent* spriteComponent;
    
    public:
        Object();
        ~Object();
        Object(Vect2D, Vect2D);
        Object(Vect2D, Vect2D, int, int);

        Rect getHitbox();
        void setPosition(int x, int y);
        void setPosition(Vect2D pos);

        virtual void update() = 0;
        void display();
};

#endif /* OBJECT_HPP */