#include "Game.hpp"

Game::Game()
{
    running = false;
    playing = false;
    highscore = 0; // load from file?
    // objects.reserve() ?
}

Game::~Game()
{
    ObjectManager::clearObjects();
}

void Game::init()
{
    running = true;

    int asteroidSize = LARGE_ASTEROID;
    ObjectManager::createObject(OBJECT_SPACESHIP, NULL);
    ObjectManager::createObject(OBJECT_ASTEROID, &asteroidSize);
    ObjectManager::createObject(OBJECT_ROCKET, NULL);
}

void Game::update()
{
    // handle events
    int a = Engine::handleEvents();

    // check if game closed
    if (a == EVT_CLOSE_GAME)
    {
        running = false;
    }

    // update objects
    ObjectManager::updateObjects();
}

void Game::display()
{
    Engine::clear();
    ObjectManager::displayObjects();    // TODO: GraphicsManager ?
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
