#ifndef SPACESHIPPHYSICSCOMPONENT_HPP
#define SPACESHIPPHYSICSCOMPONENT_HPP

#include <PhysicsComponent.hpp>

class SpaceshipPhysicsComponent : public PhysicsComponent
{
    private:
        void turn(Object*);

    public:
        void update(Object*);
};

#endif /* SPACESHIPPHYSICSCOMPONENT_HPP */