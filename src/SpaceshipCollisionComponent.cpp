#include <SpaceshipCollisionComponent.hpp>
#include <Object.hpp>

SpaceshipCollisionComponent::SpaceshipCollisionComponent()
{
    mask = COLLISION_MASK_SPACESHIP;
    collisionId = COLLISION_ID_SPACESHIP;
}

SpaceshipCollisionComponent::SpaceshipCollisionComponent(Rect& h) :
    CollisionComponent(h)
{
    mask = COLLISION_MASK_SPACESHIP;
    collisionId = COLLISION_ID_SPACESHIP;
}

void SpaceshipCollisionComponent::update(Object* o)
{
    switch(colliding)
    {
        case NO_COLLISION:
        case COLLISION_ID_SPACESHIP:
        case COLLISION_ID_ROCKET:
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
