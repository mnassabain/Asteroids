#ifndef PHYSICSCOMPONENT_HPP
#define PHYSICSCOMPONENT_HPP

#include <Engine.hpp>

class Object;

class PhysicsComponent
{
    private:
        static int WORLD_WIDTH;
        static int WORLD_HEIGHT;

        static bool outOfBounds(Object*);

    public:
        void update(Object*);
};


#endif /* PHYSICSCOMPONENT_HPP */
