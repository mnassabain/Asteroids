#include <SpaceshipPhysicsComponent.hpp>
#include <Spaceship.hpp>
#include <Object.hpp>

void SpaceshipPhysicsComponent::turn(Object* o)
{
    Spaceship* s = (Spaceship*)o;

    if (s->isTurning() == CLOCKWISE)
    {
        s->setOrientation((s->getOrientation() + 360 + 5) % 360);
    }
    else if (s->isTurning() == COUNTERCLOCKWISE)
    {
        s->setOrientation((s->getOrientation() + 360 - 5) % 360);
    }
}

void SpaceshipPhysicsComponent::update(Object* o)
{
    turn(o);

    if (o->getAcceleration() == 0)
    {
        if (o->getSpeed() != 0)
            o->setSpeed(o->getSpeed() - 1);
    }
    else
    {
        if (o->getSpeed() != Spaceship::MAX_SPEED)
            o->setSpeed(o->getSpeed() + 1);
    }

    PhysicsComponent::update(o);
}