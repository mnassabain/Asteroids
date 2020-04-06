#ifndef OBJECTMANAGER_HPP
#define OBJECTMANAGER_HPP

#define OBJECT_SPACESHIP    1
#define OBJECT_ASTEROID     2
#define OBJECT_ROCKET       3

#include <vector>
#include <CollisionComponent.hpp>

using namespace std;

class Object;

class ObjectManager
{
    private:
        static vector<Object*> objects;
        static bool collision(Object*, Object*);

    public:
        static void createObject(int, void*);
        static void clearObjects();
        static void displayObjects();
        static void updateObjects();
};

#endif /* OBJECTMANAGER_HPP */
