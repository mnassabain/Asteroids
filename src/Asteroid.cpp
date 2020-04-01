#include "Asteroid.hpp"

Asteroid::Asteroid(int size) :
    Object(new SpriteComponent("resources/asteroid.png"))
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

    orientation = 150;
    speed = 2;
}

void Asteroid::update()
{
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
