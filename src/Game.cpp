#include "Game.hpp"

Game::Game()
{
    running = false;
    playing = false;
    highscore = 0; // load from file?
    level = 0;
    score = 0;
    nbAsteroids = 0;
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

    ObjectManager::init(level);
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
    else if (a == EVT_START_GAME && level == 0)
    {
        Engine::clearEvents();
        levelUp();
    }

    // update objects
    ObjectManager::updateObjects();

    int prevScore = score;
    score += ObjectManager::getPoints();
    if (prevScore != score)
    {
        // cout << "score: " << score << endl; // HERE: uncomment to show score
        ImageManager::updateScore(score);
    }

    // int tmp2 = nbAsteroids; // HERE: uncomment 2 lines to display nbAsteroids
    nbAsteroids = ObjectManager::getNbAsteroids();
    // if (tmp2 != nbAsteroids) cout << "nb Asteroids: " << nbAsteroids << endl;
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


void Game::levelUp()
{
    level++;
    ImageManager::clearImages();
    ImageManager::init(level);
    ObjectManager::clearObjects();
    ObjectManager::init(level);
}
