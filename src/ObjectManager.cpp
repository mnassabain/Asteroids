#include <ObjectManager.hpp>
#include <Spaceship.hpp>
#include <Asteroid.hpp>
#include <Rocket.hpp>

vector<Object*> ObjectManager::objects;
void ObjectManager::createObject(int code, void* params)
{
    switch(code)
    {
        case OBJECT_SPACESHIP:
            objects.push_back(new Spaceship());
            break;
            
        case OBJECT_ASTEROID:
            {
            int asteroidSize = *(int*)params;
            objects.push_back(new Asteroid(asteroidSize));
            }
            break;

        case OBJECT_ROCKET:
            {
            Spaceship* s = (Spaceship*) params;
            Vect2D delta = 
                Vect2D(s->getDimensions().getX()/2,s->getDimensions().getY()/2);
            Vect2D startPos = s->getPosition() + delta;
            int orientation = s->getOrientation();
            objects.push_back(new Rocket(startPos, orientation));
            }
            break;
    }
}

void ObjectManager::clearObjects()
{
    vector<Object*>::iterator obj;
    for(obj = objects.begin(); obj != objects.end(); obj++)
    {
        delete(*obj);
    }
}

void ObjectManager::updateObjects()
{
    vector<Object*>::iterator end = objects.end();
    vector<Object*>::iterator obj;
    for(obj = objects.begin(); obj != end; obj++)
    {
        // if (*obj)->isDestroyed() { save next ptr; splice this elt }
        (*obj)->update();
    }
}

void ObjectManager::displayObjects()
{
    vector<Object*>::iterator obj;
    for(obj = objects.begin(); obj != objects.end(); obj++)
    {
        (*obj)->display();
    }
}
