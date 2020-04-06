#include "Object.hpp"

Object::Object(SpriteComponent* s)
{
    active = true;
    orientation = 0;
    speed = 0;
    acceleration = 0;
    spriteComponent = s;
    inputComponent = NULL;
    physicsComponent = NULL;
    collisionComponent = NULL;
}

Object::Object(SpriteComponent* s, InputComponent* i)
{
    active = true;
    orientation = 0;
    speed = 0;
    acceleration = 0;
    spriteComponent = s;
    inputComponent = i;
    physicsComponent = NULL;
    collisionComponent = NULL;
}

Object::Object(SpriteComponent* s, InputComponent* i, PhysicsComponent* p)
{
    active = true;
    orientation = 0;
    speed = 0;
    acceleration = 0;
    spriteComponent = s;
    inputComponent = i;
    physicsComponent = p;
    collisionComponent = NULL;
}

Object::Object(SpriteComponent* s, InputComponent* i, PhysicsComponent* p, 
    CollisionComponent* c)
{
    active = true;
    orientation = 0;
    speed = 0;
    acceleration = 0;
    spriteComponent = s;
    inputComponent = i;
    physicsComponent = p;
    collisionComponent = c;
}

Object::~Object()
{
    delete spriteComponent;
    delete inputComponent;
    delete physicsComponent;
}

Object& Object::operator= (const Object &o)
{
    hitbox = o.hitbox;
    orientation = o.orientation;
    speed = o.speed;
    acceleration = o.acceleration;
    active = o.active;

    delete spriteComponent;
    spriteComponent = o.spriteComponent;
    delete inputComponent;
    inputComponent = o.inputComponent;
    delete physicsComponent;
    physicsComponent = o.physicsComponent;

    return *this;
}

Rect Object::getHitbox()
{
    return hitbox;
}

int Object::getOrientation()
{
    return orientation;
}

void Object::setOrientation(int o)
{
    orientation = o;
}

int Object::getSpeed()
{
    return speed;
}

void Object::setSpeed(int s)
{
    speed = s;
}

int Object::getAcceleration()
{
    return acceleration;
}

Vect2D Object::getPosition()
{
    return hitbox.getPosition();
}

void Object::setPosition(int x, int y)
{
    hitbox.setPosition(x, y);
}

void Object::setPosition(Vect2D pos)
{
    hitbox.setPosition(pos);
}

Vect2D Object::getDimensions()
{
    return hitbox.getDimensions();
}

void Object::setDimensions(int x, int y)
{
    hitbox.setDimensions(x, y);
}

void Object::setDimensions(Vect2D dim)
{
    hitbox.setDimensions(dim);
}

int Object::getX()
{
    return getPosition().getX();
}

int Object::getY()
{
    return getPosition().getY();
}

int Object::getW()
{
    return getDimensions().getX();
}

int Object::getH()
{
    return getDimensions().getY();
}

void Object::display()
{
    spriteComponent->draw(this);
}

void Object::destroy()
{
    active = false;
}

bool Object::isDestroyed()
{
    return !active;
}

bool Object::collidingWith(Object* o)
{
    return collisionComponent->isCollidingWith(o);
}

CollisionComponent* Object::getCollisionComponent()
{
    return collisionComponent;
}
