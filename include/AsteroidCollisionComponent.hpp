#ifndef ASTEROIDCOLLISIONCOMPONENT_HPP
#define ASTEROIDCOLLISIONCOMPONENT_HPP

#include <CollisionComponent.hpp>

class AsteroidCollisionComponent : public CollisionComponent
{
    public:
        AsteroidCollisionComponent();
        AsteroidCollisionComponent(Rect&);
};

#endif /* ASTEROIDCOLLISIONCOMPONENT_HPP */
