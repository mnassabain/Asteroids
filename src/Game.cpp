#include "Game.hpp"

Game::Game()
{
    running = false;
    playing = false;
    highscore = 0; // load from file?
    level = 0;
    score = 0;
    extraLifeCounter = 0;
    nbAsteroids = 0;
    lives = 3;
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
    score = 0;
    lives = 3;
    nbAsteroids = 0;

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

    if (level != 0 && !ObjectManager::isSpaceshipActive())
    {
        if (lives == 1)
        {
            gameOver();
            if (score > highscore) highscore = score;
            score = 0;
            extraLifeCounter = 0;
            lives = 3;
            nbAsteroids = 0;
        }
        else
        {
            lives--;
            ObjectManager::createObject(OBJECT_SPACESHIP, NULL);
        }
    }

    // int prevScore = score;
    int newPoints = ObjectManager::getPoints();
    score += newPoints;
    extraLifeCounter += newPoints;
    if (extraLifeCounter >= 1000)
    {
        if (lives < MAX_LIVES) lives++;
        extraLifeCounter -= 1000;
    }

    // int tmp2 = nbAsteroids; // HERE: uncomment 2 lines to display nbAsteroids
    nbAsteroids = ObjectManager::getNbAsteroids();
    if (nbAsteroids == 0)
        levelUp();
    // if (tmp2 != nbAsteroids) cout << "nb Asteroids: " << nbAsteroids << endl;
}

void Game::display()
{
    Engine::clear();
    ObjectManager::displayObjects();    // TODO: GraphicsManager ?
    ImageManager::displayImages();
    if (level != 0)
    {
        ImageManager::displayScore(score);
        ImageManager::displayLives(lives);
    }
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

void Game::gameOver()
{
    level = 0;
    ImageManager::clearImages();
    ImageManager::init(level);
    ObjectManager::clearObjects();
    ObjectManager::init(level);
}
