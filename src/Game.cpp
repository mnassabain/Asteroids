#include "Game.hpp"

Game::Game()
{
    running = false;
    playing = false;
    highscore = 0; // load from file?
    // objects.reserve()..
}

void Game::init()
{
    running = true;
}

void Game::update()
{
    int a = Engine::handleEvents();
    if (a == CLOSE_GAME_EVENT)
    {
        running = false;
    }
}

void Game::display()
{
    Engine::clear();
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
