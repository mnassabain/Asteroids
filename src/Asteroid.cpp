#include "Asteroid.hpp"

Asteroid::Asteroid(int size) :
    Object(new SpriteComponent("resources/asteroid.png"),
    NULL, new PhysicsComponent(), new AsteroidCollisionComponent())
{
    this->size = size;
    init();
}

Asteroid::~Asteroid()
{}

void Asteroid::init()
{
    hitbox.setPosition(physicsComponent->getRandomPosition());
    if (size == LARGE_ASTEROID)
    {
        // TODO: remplacer par valeurs precises + macro
        hitbox.setDimensions(80, 80);
    }
    else if (size == MEDIUM_ASTEROID)
    {
        hitbox.setDimensions(50, 50);
    }
    else
    {
        hitbox.setDimensions(30, 30);
    }
    
    Rect colliderBox(
        hitbox.x(),
        hitbox.y(),
        hitbox.w(),
        hitbox.h()
    );
    collisionComponent->setHitbox(colliderBox);

    orientation = RandomGenerator::generate() % 360;
    speed = 2;
}

void Asteroid::update()
{
    physicsComponent->update(this);

    int dx, dy;
    double alpha = (orientation % 90) * M_PI / 180;
    if (0 <= orientation && orientation < 90)
    {
        dx = sin(alpha) * speed * 1.7;
        dy = -cos(alpha) * speed * 1.7;
    }
    else if (90 <= orientation && orientation < 180)
    {
        dx = cos(alpha) * speed * 1.7;
        dy = sin(alpha) * speed * 1.7;
    }
    else if (180 <= orientation && orientation < 270)
    {
        dx = -sin(alpha) * speed * 1.7;
        dy = cos(alpha) * speed * 1.7;
    }
    else
    {
        dx = -cos(alpha) * speed * 1.7;
        dy = -sin(alpha) * speed * 1.7;
    }

    setPosition(getPosition() + Vect2D(dx, dy));
}

int Asteroid::getSize()
{
    return size;
}
