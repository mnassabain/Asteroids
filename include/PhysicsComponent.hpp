#ifndef PHYSICSCOMPONENT_HPP
#define PHYSICSCOMPONENT_HPP

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

    public:
        virtual ~PhysicsComponent();
        static Vect2D getRandomPosition();
        virtual void update(Object*);
};


#endif /* PHYSICSCOMPONENT_HPP */
