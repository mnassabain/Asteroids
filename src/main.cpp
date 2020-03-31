#include <iostream>

#include "Vect2D.hpp"
#include "Asteroid.hpp"
#include <Game.hpp>
#include <Engine.hpp>

int main()
{
    Engine::init();

    // game loop
    Game game;
    game.init();
    while(game.isRunning())
    {
        game.update();
        game.display();
        // TODO: add delay to control fps
    }

    Engine::destroy();

    return 0;
}
