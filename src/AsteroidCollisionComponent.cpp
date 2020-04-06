#include <AsteroidCollisionComponent.hpp>
#include <Object.hpp>

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

void AsteroidCollisionComponent::update(Object* o)
{
    switch(colliding)
    {
        case NO_COLLISION:
        case COLLISION_ID_ASTEROID:
        case COLLISION_ID_SPACESHIP:
        default:
            // nothing
            colliding = NO_COLLISION;
            break;

        case COLLISION_ID_ROCKET:
            o->destroy();
            colliding = NO_COLLISION;
            break;
    }
}
