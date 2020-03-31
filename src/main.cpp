#include <iostream>

#include "Vect2D.hpp"
#include "Asteroid.hpp"
#include "Game.hpp"

int main()
{
    // tests
    Vect2D v1(1, 0);
    Vect2D v2(5, 7);

    Vect2D v3 = v1 + v2;

    std::cout << "(" << v3.getX() << ", " << v3.getY() << ")" << std::endl;

    Asteroid a(LARGE_ASTEROID);

    // game loop
    Game game;
    game.init();
    while(game.isRunning())
    {
        game.update();
        game.display();
        // TODO: add delay to control fps
    }

    return 0;
}