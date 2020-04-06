#include <AsteroidCollisionComponent.hpp>

AsteroidCollisionComponent::AsteroidCollisionComponent()
{
    mask = COLLISION_MASK_ASTEROID;
    collisionId = COLLISION_ID_ASTEROID;
}

AsteroidCollisionComponent::AsteroidCollisionComponent(Rect& h) :
    CollisionComponent(h)
{
    mask = COLLISION_MASK_ASTEROID;
    collisionId = COLLISION_ID_ASTEROID;
}
