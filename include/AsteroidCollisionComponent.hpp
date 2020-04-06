#ifndef ASTEROIDCOLLISIONCOMPONENT_HPP
#define ASTEROIDCOLLISIONCOMPONENT_HPP

#include <CollisionComponent.hpp>

class AsteroidCollisionComponent : public CollisionComponent
{
    public:
        AsteroidCollisionComponent();
        AsteroidCollisionComponent(Rect&);
        void update(Object*);
};

#endif /* ASTEROIDCOLLISIONCOMPONENT_HPP */
