#include <RocketCollisionComponent.hpp>
#include <Object.hpp>

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

void RocketCollisionComponent::update(Object* o)
{
    switch(colliding)
    {
        case NO_COLLISION:
        case COLLISION_ID_ROCKET:
        case COLLISION_ID_SPACESHIP:
        default:
            // nothing
            colliding = NO_COLLISION;
            break;

        case COLLISION_ID_ASTEROID:
            o->destroy();
            colliding = NO_COLLISION;
            break;
    }
}
