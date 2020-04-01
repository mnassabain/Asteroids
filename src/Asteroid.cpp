#include "Asteroid.hpp"

Asteroid::Asteroid(int size) : Object(new AsteroidSpriteComponent())
{
    this->size = size;
    init();
}

void Asteroid::init()
{
    // TODO: remplacer par valeurs aléatoires, ou les passer en arg?? 
    hitbox.setPosition(0, 0);
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
}

void Asteroid::update()
{
}

int Asteroid::getSize()
{
    return size;
}
