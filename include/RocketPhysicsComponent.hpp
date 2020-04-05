#ifndef ROCKETPHYSICSCOMPONENT_HPP
#define ROCKETPHYSICSCOMPONENT_HPP

#include <PhysicsComponent.hpp>

class RocketPhysicsComponent : public PhysicsComponent
{
    public:
        void update(Object*);
};

#endif /* ROCKETPHYSICSCOMPONENT_HPP */