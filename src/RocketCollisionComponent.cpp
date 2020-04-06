#include <RocketCollisionComponent.hpp>

RocketCollisionComponent::RocketCollisionComponent()
{
    mask = COLLISION_MASK_ROCKET;
    collisionId = COLLISION_ID_ROCKET;
}
