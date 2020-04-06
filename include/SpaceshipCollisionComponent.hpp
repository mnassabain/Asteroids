#ifndef SPACESHIPCOLLISIONCOMPONENT_HPP
#define SPACESHIPCOLLISIONCOMPONENT_HPP

#include <CollisionComponent.hpp>

class SpaceshipCollisionComponent : public CollisionComponent
{
    public:
        SpaceshipCollisionComponent();
        SpaceshipCollisionComponent(Rect&);

        void update(Object*);
};

#endif /* SPACESHIPCOLLISIONCOMPONENT_HPP */
