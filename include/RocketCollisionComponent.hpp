#ifndef ROCKETCOLLISIONCOMPONENT_HPP
#define ROCKETCOLLISIONCOMPONENT_HPP

#include <CollisionComponent.hpp>

class RocketCollisionComponent : public CollisionComponent
{
    public:
        RocketCollisionComponent();
        RocketCollisionComponent(Rect&);
        void update(Object*);
};

#endif /* ROCKETCOLLISIONCOMPONENT_HPP */
