#include "Asteroid.hpp"

Asteroid::Asteroid(int size) :
    Object(new AsteroidSpriteComponent(),
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
    collisionComponent->update(this);
}

int Asteroid::getSize()
{
    return size;
}
