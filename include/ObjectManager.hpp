#ifndef OBJECTMANAGER_HPP
#define OBJECTMANAGER_HPP

#define OBJECT_SPACESHIP    1
#define OBJECT_ASTEROID     2
#define OBJECT_ROCKET       3

#include <vector>
#include <algorithm>
#include <CollisionComponent.hpp>

using namespace std;

class Object;

class ObjectManager
{
    private:
        static vector<Object*> objects;
        static bool collision(Object*, Object*);
        static int points; // points collected during current frame
        static int nbAsteroids; // current nb of asteroids

    public:
        static void init(int);
        static void createObject(int, void*);
        static void clearObjects();
        static void displayObjects();
        static void updateObjects();
        static void addPoints(int);
        static int getPoints();
        static void removeAsteroid();
        static int getNbAsteroids();
};

#endif /* OBJECTMANAGER_HPP */
