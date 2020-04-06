#ifndef ROCKETCOLLISIONCOMPONENT_HPP
#define ROCKETCOLLISIONCOMPONENT_HPP

#include <CollisionComponent.hpp>

class RocketCollisionComponent : public CollisionComponent
{
    public:
        RocketCollisionComponent();
        RocketCollisionComponent(Rect&);
};

#endif /* ROCKETCOLLISIONCOMPONENT_HPP */
