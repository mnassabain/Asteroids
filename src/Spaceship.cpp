#include "Spaceship.hpp"

Spaceship::Spaceship() : 
    Object(new SpriteComponent("resources/player.png"), new InputComponent(),
        new SpaceshipPhysicsComponent(), new SpaceshipCollisionComponent()
    )
{
    shooting = false;
    setDimensions(50, 75);

    Rect colliderBox(
        hitbox.x(),
        hitbox.y(),
        hitbox.w(),
        hitbox.h()
    );
    collisionComponent->setHitbox(colliderBox);
}

Spaceship::~Spaceship()
{}

void Spaceship::update()
{
    inputComponent->update(this);
    physicsComponent->update(this);
    collisionComponent->update(this);
    if (shooting)
    {
        shoot();
        shooting = false;
    }
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

void Spaceship::shoot()
{
    ObjectManager::createObject(OBJECT_ROCKET, this);
}

void Spaceship::startTurning(int direction)
{
    turning = direction;
}

void Spaceship::stopTurning()
{
    turning = 0;
}

int Spaceship::isTurning()
{
    return turning;
}
