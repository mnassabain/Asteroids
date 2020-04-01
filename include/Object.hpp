#ifndef OBJECT_HPP
#define OBJECT_HPP

#define _USE_MATH_DEFINES

#include <cmath>

#include <Vect2D.hpp>
#include <Rect.hpp>
#include <SpriteComponent.hpp>
#include <InputComponent.hpp>

class Object
{
    protected:
        Rect hitbox;
        int orientation;    // 0 - 360
        int speed;          // 0 - 100
        int acceleration;   // 0 (no acc), 1 (acceleration)

        SpriteComponent* spriteComponent;
        InputComponent* inputComponent;
    
    public:
        Object(SpriteComponent*);
        Object(SpriteComponent*, InputComponent*);
        virtual ~Object();
        Object(Vect2D, Vect2D);
        Object(Vect2D, Vect2D, int, int);

        Rect getHitbox();
        Vect2D getPosition();
        void setPosition(int x, int y);
        void setPosition(Vect2D pos);
        Vect2D getDimensions();
        void setDimensions(int x, int y);
        void setDimensions(Vect2D dim);

        int getOrientation();

        virtual void update() = 0;
        void display();
};

#endif /* OBJECT_HPP */