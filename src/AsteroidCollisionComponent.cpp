#include <AsteroidCollisionComponent.hpp>

AsteroidCollisionComponent::AsteroidCollisionComponent()
{
    mask = COLLISION_MASK_ASTEROID;
    collisionId = COLLISION_ID_ASTEROID;
}
