#ifndef PHYSICSCOMPONENT_HPP
#define PHYSICSCOMPONENT_HPP

#define _USE_MATH_DEFINES

#include <cmath>

#include <RandomGenerator.hpp>
#include <Vect2D.hpp>
#include <Engine.hpp>

class Object;

class PhysicsComponent
{
    protected:
        static int WORLD_WIDTH;
        static int WORLD_HEIGHT;

        static bool outOfBounds(Object*);

        void move(Object*);

    public:
        PhysicsComponent() {}
        PhysicsComponent(const PhysicsComponent&) {}
        virtual ~PhysicsComponent();
        static Vect2D getRandomPosition();
        virtual void update(Object*);
};


#endif /* PHYSICSCOMPONENT_HPP */
