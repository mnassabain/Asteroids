#include "Spaceship.hpp"

Spaceship::Spaceship() : Object(new SpaceshipSpriteComponent(), new InputComponent())
{}

void Spaceship::update()
{
    inputComponent->update(this);
    turn();
    move();
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

void Spaceship::startTurning(int direction)
{
    turning = direction;
}

void Spaceship::stopTurning()
{
    turning = 0;
}

void Spaceship::turn()
{
    if (turning == CLOCKWISE)
    {
        orientation = (orientation + 360 + 2) % 360;
    }
    else if (turning == COUNTERCLOCKWISE)
    {
        orientation = (orientation + 360 - 2) % 360;
    }
}

void Spaceship::move()
{
    if (acceleration == 0)
    {
        speed == 0 ? 0 : speed--;
    }
    else
    {
        speed == MAX_SPEED ? speed : speed++;
    }

    int dx, dy;
    
    double alpha = (orientation % 90) * M_PI / 180;
    if (0 <= orientation && orientation < 90)
    {
        dx = sin(alpha) * speed;
        dy = -cos(alpha) * speed;
    }
    else if (90 <= orientation && orientation < 180)
    {
        dx = cos(alpha) * speed;
        dy = sin(alpha) * speed;
    }
    else if (180 <= orientation && orientation < 270)
    {
        dx = -sin(alpha) * speed;
        dy = cos(alpha) * speed;
    }
    else
    {
        dx = -cos(alpha) * speed;
        dy = -sin(alpha) * speed;
    }

    setPosition(getPosition() + Vect2D(dx, dy));
}
