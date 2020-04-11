#include "Spaceship.hpp"

Spaceship::Spaceship() : 
    Object(new SpaceshipSpriteComponent(), new InputComponent(),
        new SpaceshipPhysicsComponent(), new SpaceshipCollisionComponent()
    )
{
    shooting = false;
    reload = 0;
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
        if (reload == 0)
        {
            shoot();
            reload = RELOAD_FRAMES;
        }
        else
            reload--;
    }   
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

void Spaceship::teleport()
{
    int x = (RandomGenerator::generate() + SCREEN_WIDTH) % SCREEN_WIDTH;
    int y = (RandomGenerator::generate() + SCREEN_HEIGHT) % SCREEN_HEIGHT;

    Vect2D oldObjPos = getPosition();
    Rect oldBox = collisionComponent->getHitbox();
    
    int dx = oldBox.x() - oldObjPos.getX();
    int dy = oldBox.y() - oldObjPos.getY();

    setPosition(x, y);

    oldBox.x(x + dx);
    oldBox.y(y + dy);

    collisionComponent->setHitbox(oldBox);
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
