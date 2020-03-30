#include "Object.hpp"

Object::Object()
{
    orientation = 0;
    speed = 0;
    acceleration = 0;
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