#ifndef GAME_H
#define GAME_H

#include <vector>
#include "Object.hpp"

class Game
{
    private:
        bool running;   // indique si le jeu est lancé - true meme si ecran menu
        bool playing;   // indique si on joue - on est dans l'écran de jeu

        int highscore;
        int level;      // savoir le niveau

        std::vector<Object*> objects;    // contenir objets triés par pos.x

        void startPlaying();
        void stopPlaying();

    public:
        Game();

        void init();
        void update();
        void display();

        bool isRunning();
};

#endif /* GAME_H */