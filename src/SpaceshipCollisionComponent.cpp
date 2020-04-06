#include <SpaceshipCollisionComponent.hpp>

SpaceshipCollisionComponent::SpaceshipCollisionComponent()
{
    mask = COLLISION_MASK_SPACESHIP;
    collisionId = COLLISION_ID_SPACESHIP;
}
