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

void Rect::setPosition(Vect2D pos)
{
    position = pos;
}

void Rect::setPosition(int x, int y)
{
    position = Vect2D(x, y);
}

Vect2D Rect::getDimensions()
{
    return dimensions;
}

void Rect::setDimensions(Vect2D dim)
{
    dimensions = dim;
}

void Rect::setDimensions(int w, int h)
{
    dimensions = Vect2D(w, h);
}

void Rect::setWidth(int w)
{
    dimensions.setX(w);
}

void Rect::setHeight(int h)
{
    dimensions.setY(h);
}

void Rect::set(int x, int y, int w, int h)
{
    position.setX(x);
    position.setY(y);
    dimensions.setX(w);
    dimensions.setY(h);
}
