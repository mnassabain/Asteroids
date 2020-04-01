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
    vector<Object*>::iterator obj;
    for(obj = objects.begin(); obj != objects.end(); obj++)
    {
        delete (*obj);
    }
}

void Game::init()
{
    running = true;

    objects.push_back(new Spaceship());
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
    vector<Object*>::iterator obj;
    for(obj = objects.begin(); obj != objects.end(); obj++)
    {
        (*obj)->update();
    }
}

void Game::display()
{
    Engine::clear();
    vector<Object*>::iterator obj;
    for(obj = objects.begin(); obj != objects.end(); obj++)
    {
        (*obj)->display();
    }
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
