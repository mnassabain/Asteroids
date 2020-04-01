#ifndef SPACESHIP_HPP
#define SPACESHIP_HPP

#include "Vect2D.hpp"
#include "Object.hpp"
#include <SpaceshipSpriteComponent.hpp>

#define PLAYER_MAX_LIVES   5

class Spaceship : public Object
{
    private:
        int lives;       // 0 - 5
        int score;
        int turning;    // 0: no turning, -1: ccw, 1: cw
        bool shooting;  // y/n

        static const int MAX_LIVES = PLAYER_MAX_LIVES;

    public:
        Spaceship();
        void update();

        int getScore();
        void addPoints(int points);
        void addLife();
        void removeLife();
        int getLives();
        void accelerate();
        void decelerate();
        void turn(int direction);
        void teleport(int x, int y);
        void teleport(Vect2D pos);
        void startShooting();
        void stopShooting();
};

#endif /* SPACESHIP_HPP */