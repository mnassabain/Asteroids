#include <RocketCollisionComponent.hpp>

RocketCollisionComponent::RocketCollisionComponent()
{
    mask = COLLISION_MASK_ROCKET;
    collisionId = COLLISION_ID_ROCKET;
}

RocketCollisionComponent::RocketCollisionComponent(Rect& h) :
    CollisionComponent(h)
{
    mask = COLLISION_MASK_ROCKET;
    collisionId = COLLISION_ID_ROCKET;
}
