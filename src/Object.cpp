#include "Object.hpp"

Object::Object()
{
    orientation = 0;
    speed = 0;
    acceleration = 0;
}

Rect Object::getHitbox()
{
    return hitbox;
}