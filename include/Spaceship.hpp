#ifndef SPACESHIP_HPP
#define SPACESHIP_HPP

#include <Vect2D.hpp>
#include <Object.hpp>
#include <ObjectManager.hpp>
#include <SpaceshipCollisionComponent.hpp>
#include <SpaceshipPhysicsComponent.hpp>
#include <SpaceshipSpriteComponent.hpp>
#include <RandomGenerator.hpp>

#define PLAYER_MAX_LIVES    5
#define CLOCKWISE           1
#define COUNTERCLOCKWISE    -1

class Spaceship : public Object
{
    private:
        int lives;       // 0 - 5
        int turning;    // 0: no turning, -1: ccw, 1: cw
        bool shooting;  // y/n

        int reload;     // reload frames
        static const int RELOAD_FRAMES = 8; // x frames to reload

    public:
        static const int MAX_LIVES = PLAYER_MAX_LIVES;
        static const int MAX_SPEED = 3;

        Spaceship();
        ~Spaceship();
        void update();

        void addLife();
        void removeLife();
        int getLives();
        void accelerate();
        void decelerate();
        void startTurning(int direction);
        void stopTurning();
        int isTurning(); 
        void teleport();
        void startShooting();
        void stopShooting();
        void shoot();
        void destroy();

};

#endif /* SPACESHIP_HPP */