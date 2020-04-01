#include "Object.hpp"

Object::Object(SpriteComponent* s)
{
    orientation = 0;
    speed = 0;
    acceleration = 0;
    spriteComponent = s;
    inputComponent = NULL;
}

Object::Object(SpriteComponent* s, InputComponent* i)
{
    orientation = 0;
    speed = 0;
    acceleration = 0;
    spriteComponent = s;
    inputComponent = i;
}

Object::~Object()
{
    delete spriteComponent;
}

Object::Object(Vect2D pos, Vect2D dim) : hitbox(pos, dim)
{
    orientation = 0;
    speed = 0;
    acceleration = 0;
}

Object::Object(Vect2D pos, Vect2D dim, int s, int o) : hitbox(pos, dim)
{
    speed = s;
    orientation = o;
    acceleration = 0;
}

Rect Object::getHitbox()
{
    return hitbox;
}

int Object::getOrientation()
{
    return orientation;
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

void Object::display()
{
    spriteComponent->draw(this);
}
