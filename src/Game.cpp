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

}

void Game::display()
{
    Engine::draw();
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
