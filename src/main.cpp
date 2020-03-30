#include <iostream>

#include "Vect2D.hpp"
#include "Asteroid.hpp"

int main()
{
    Vect2D v1(1, 0);
    Vect2D v2(5, 7);

    Vect2D v3 = v1 + v2;

    std::cout << "(" << v3.getX() << ", " << v3.getY() << ")" << std::endl;

    Asteroid a(LARGE_ASTEROID);

    return 0;
}