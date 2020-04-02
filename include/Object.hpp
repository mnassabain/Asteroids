#ifndef OBJECT_HPP
#define OBJECT_HPP

#define _USE_MATH_DEFINES

#include <cmath>

#include <Vect2D.hpp>
#include <Rect.hpp>
#include <SpriteComponent.hpp>
#include <InputComponent.hpp>
#include <PhysicsComponent.hpp>

class Object
{
    protected:
        Rect hitbox;
        int orientation;    // 0 - 360
        int speed;          // 0 - 100
        int acceleration;   // 0 (no acc), 1 (acceleration)
        bool active;        // if false, object is destroyed

        SpriteComponent* spriteComponent;
        InputComponent* inputComponent;
        PhysicsComponent* physicsComponent;
    
    public:
        Object(SpriteComponent*);
        Object(SpriteComponent*, InputComponent*);
        Object(SpriteComponent*, InputComponent*, PhysicsComponent*);
        virtual ~Object();

        Object& operator= (const Object &o);

        Rect getHitbox();
        Vect2D getPosition();
        void setPosition(int x, int y);
        void setPosition(Vect2D pos);
        Vect2D getDimensions();
        void setDimensions(int x, int y);
        void setDimensions(Vect2D dim);

        int getX();
        int getY();
        int getW();
        int getH();

        void destroy();
        bool isDestroyed();

        int getOrientation();

        virtual void update() = 0;
        void display();
};

#endif /* OBJECT_HPP */