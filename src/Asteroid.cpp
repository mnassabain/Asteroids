#include "Asteroid.hpp"

Asteroid::Asteroid(int size)
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
        hitbox.setDimensions(10, 10);
    }
    else if (size == MEDIUM_ASTEROID)
    {
        hitbox.setDimensions(6, 6);
    }
    else
    {
        hitbox.setDimensions(4, 4);
    }
}

void Asteroid::destroy()
{
    size--;
    hitbox.set(0, 0, 0, 0);
    if (size != DESTROYED_ASTEROID)
    {
        // créer 2 asteroides et renvoyer ... à coder
    }
}
