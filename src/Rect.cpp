#include "Rect.hpp"

Rect::Rect(const Rect& r)
{
    x(r.x());
    y(r.y());
    w(r.w());
    h(r.h());
}

Rect::Rect(Vect2D pos, Vect2D dim): position(pos), dimensions(dim)
{}

Rect::Rect(int x, int y, int w, int h) : position(x, y), dimensions(w, h)
{}

Rect& Rect::operator= (const Rect& r)
{
    x(r.x());
    y(r.y());
    w(r.w());
    h(r.h());

    return *this;
}

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

int Rect::x() const
{
    return position.getX();
}

int Rect::y() const
{
    return position.getY();
}

int Rect::w() const
{
    return dimensions.getX();
}

int Rect::h() const
{
    return dimensions.getY();
}

void Rect::x(int dx)
{
    position.setX(dx);
}

void Rect::y(int dy)
{
    position.setY(dy);

}

void Rect::w(int dw)
{
    dimensions.setX(dw);
}

void Rect::h(int dh)
{
    dimensions.setY(dh);
}

        