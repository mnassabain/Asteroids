#include "Spaceship.hpp"

Spaceship::Spaceship() : Object(new SpaceshipSpriteComponent(), new InputComponent())
{}

void Spaceship::update()
{
    inputComponent->update(this);
}

int Spaceship::getScore()
{
    return score;
}

void Spaceship::addPoints(int points)
{
    score += points;
}

void Spaceship::addLife()
{
    if (lives < MAX_LIVES)
    {
        lives++;
    }
}

void Spaceship::removeLife()
{
    lives--;
}

int Spaceship::getLives()
{
    return lives;
}

void Spaceship::accelerate()
{
    acceleration = 1;
}

void Spaceship::decelerate()
{
    acceleration = 0;
}

void Spaceship::turn(int direction)
{
    turning = direction;
}

void Spaceship::teleport(int x, int y)
{
    setPosition(x, y);
}

void Spaceship::teleport(Vect2D pos)
{
    setPosition(pos);
}

void Spaceship::startShooting()
{
    shooting = true;
}

void Spaceship::stopShooting()
{
    shooting = false;
}

