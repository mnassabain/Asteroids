#include <PhysicsComponent.hpp>
#include <Object.hpp>

int PhysicsComponent::WORLD_WIDTH = SCREEN_WIDTH;
int PhysicsComponent::WORLD_HEIGHT = SCREEN_HEIGHT;

PhysicsComponent::~PhysicsComponent()
{}

bool PhysicsComponent::outOfBounds(Object* o)
{
    return (o->getX() > WORLD_WIDTH)
        || (o->getX() + o->getW() < 0)
        || (o->getY() > WORLD_HEIGHT)
        || (o->getY() + o->getH() < 0);
}

void PhysicsComponent::move(Object* o)
{
    int dx, dy;
    double alpha = (o->getOrientation() % 90) * M_PI / 180;
    if (0 <= o->getOrientation() && o->getOrientation() < 90)
    {
        dx = sin(alpha) * o->getSpeed() * 2;
        dy = -cos(alpha) * o->getSpeed() * 2;
    }
    else if (90 <= o->getOrientation() && o->getOrientation() < 180)
    {
        dx = cos(alpha) * o->getSpeed() * 2;
        dy = sin(alpha) * o->getSpeed() * 2;
    }
    else if (180 <= o->getOrientation() && o->getOrientation() < 270)
    {
        dx = -sin(alpha) * o->getSpeed() * 2;
        dy = cos(alpha) * o->getSpeed() * 2;
    }
    else
    {
        dx = -cos(alpha) * o->getSpeed() * 2;
        dy = -sin(alpha) * o->getSpeed() * 2;
    }

    o->setPosition(o->getPosition() + Vect2D(dx, dy));
}

void PhysicsComponent::update(Object* o)
{
    move(o);

    if (outOfBounds(o))
    {
        // circular world
        o->setPosition((o->getX() + WORLD_WIDTH) % WORLD_WIDTH, 
            (o->getY() + WORLD_HEIGHT) % WORLD_HEIGHT);
    }
}

Vect2D PhysicsComponent::getRandomPosition()
{
    return Vect2D(RandomGenerator::generate() % WORLD_WIDTH,
        RandomGenerator::generate() % WORLD_HEIGHT);
}
