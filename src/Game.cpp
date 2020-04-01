#include "Game.hpp"

Game::Game()
{
    running = false;
    playing = false;
    highscore = 0; // load from file?
    // objects.reserve()..

    // player = new Spaceship(PlayerSpriteComponent, InputComponent);
}

void Game::init()
{
    running = true;
}

void Game::update()
{
    // handle events
    int a = Engine::handleEvents();
    if (a == EVT_CLOSE_GAME)
    {
        running = false;
    }

    // check if game closed

    // update objects
    player.update();
}

void Game::display()
{
    Engine::clear();
    player.display();
    Engine::render();
}

bool Game::isRunning()
{
    return running;
}

void Game::startPlaying()
{
    level = 1;
    playing = true;
    // setPlayerScore(0); startLevel(); generateAsteroids();
}

void Game::stopPlaying()
{
    playing = false;
}
