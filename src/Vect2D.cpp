#include "Vect2D.hpp"

Vect2D::Vect2D()
{
    x = 0;
    y = 0;
}

Vect2D::Vect2D(int x, int y)
{
    this->x = x;
    this->y = y;
}

Vect2D::Vect2D(const Vect2D &v)
{
    x = v.x;
    y = v.y;
}

int Vect2D::getX() const
{
    return x;
}

void Vect2D::setX(int x)
{
    this->x = x;
}

int Vect2D::getY() const
{
    return y;
}

void Vect2D::setY(int y)
{
    this->y = y;
}

Vect2D Vect2D::operator+ (Vect2D const &vector)
{
    Vect2D result; 
    result.x = x + vector.x; 
    result.y = y + vector.y; 
    return result;
}

Vect2D Vect2D::operator- (Vect2D const &vector)
{
    Vect2D result; 
    result.x = x - vector.x; 
    result.y = y - vector.y; 
    return result;
}

Vect2D& Vect2D::operator= (const Vect2D &vector)
{
    x = vector.x;
    y = vector.y;
 
    return *this;
}