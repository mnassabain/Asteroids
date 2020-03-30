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

int Vect2D::getX()
{
    return x;
}

void Vect2D::setX(int x)
{
    this->x = x;
}

int Vect2D::getY()
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