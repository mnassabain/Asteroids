#include "Rect.hpp"

Vect2D Rect::getPosition()
{
    return position;
}

int Rect::getWidth()
{
    return dimensions.getX();
}

int Rect::getHeight()
{
    return dimensions.getY();
}

void Rect::setPosition(Vect2D position)
{
    this->position = position;
}

void Rect::setWidth(int w)
{
    dimensions.setX(w);
}

void Rect::setHeight(int h)
{
    dimensions.setY(h);
}