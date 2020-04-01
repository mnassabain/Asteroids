#include "Object.hpp"

Object::Object(SpriteComponent* s)
{
    orientation = 0;
    speed = 0;
    acceleration = 0;
    spriteComponent = s;
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
    spriteComponent->draw();
}
