#include "Rect.hpp"

Rect::Rect(Vect2D pos, Vect2D dim): position(pos), dimensions(dim)
{}

Rect::Rect(int x, int y, int w, int h) : position(x, y), dimensions(w, h)
{}

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