#ifndef GAME_H
#define GAME_H

#include <vector>
#include <EventCodes.hpp>
#include <Object.hpp>
#include <ObjectManager.hpp>
#include <ImageManager.hpp>
#include <Engine.hpp>
#include <Asteroid.hpp>
#include <Spaceship.hpp>
#include <Rocket.hpp>


using namespace std;

class Game
{
    private:
        bool running;   // indique si le jeu est lancé - true meme si ecran menu
        bool playing;   // indique si on joue - on est dans l'écran de jeu

        int highscore;
        int level;      // savoir le niveau

        void startPlaying();
        void stopPlaying();

    public:
        Game();
        ~Game();

        void init();
        void update();
        void display();
        void levelUp();

        bool isRunning();
};

#endif /* GAME_H */