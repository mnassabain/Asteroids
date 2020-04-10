#include "Game.hpp"

Game::Game()
{
    running = false;
    playing = false;
    highscore = 0; // load from file?
    level = 0;
    // objects.reserve() ?
}

Game::~Game()
{
    ImageManager::clearImages();
    ObjectManager::clearObjects();
}

void Game::init()
{
    running = true;

    // ObjectManager::createObject(OBJECT_SPACESHIP, NULL);
    for (int i = 0; i < 4; i++)
    {
        ObjectManager::createObject(OBJECT_ASTEROID, NULL);
    }

    ImageManager::init(level);
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
    ImageManager::displayImages();
    Engine::render();
    Engine::manageFrames();
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
