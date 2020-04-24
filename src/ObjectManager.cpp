#include <ObjectManager.hpp>
#include <Spaceship.hpp>
#include <Asteroid.hpp>
#include <Rocket.hpp>

vector<Object*> ObjectManager::objects;
int ObjectManager::nbAsteroids = 0;
int ObjectManager::points = 0;
bool ObjectManager::spaceshipActive = false;

void ObjectManager::createObject(int code, void* params)
{
    switch(code)
    {
        case OBJECT_SPACESHIP:
            objects.push_back(new Spaceship());
            spaceshipActive = true;
            break;
            
        case OBJECT_ASTEROID:
            {
                Asteroid* parentAst = (Asteroid*)params;
                if (parentAst != NULL)
                {
                    objects.push_back(
                        new Asteroid(
                            parentAst->getSize() - 1,
                            parentAst->getPosition()
                        )
                    );
                }
                else
                    objects.push_back(new Asteroid(LARGE_ASTEROID));
                nbAsteroids++;
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
    objects.clear();
    nbAsteroids = 0;
}

void ObjectManager::updateObjects()
{
    vector<int> markedForDelete;

    int size = objects.size();
    // vector<Object*>::iterator end = objects.end();
    vector<Object*>::iterator obj;
    for(int i = 0; i < size; ++i)
    {
        if (objects[i]->isDestroyed())
        {
            markedForDelete.push_back(i);
        }
        else
        {
            // check collisions
            int counter = 1;
            while(counter < size - i)
            {
                if (!objects[i + counter]->isDestroyed())
                {
                    objects[i]->collidingWith(objects[i + counter]);
                }
                counter++;
            }
            // update
            objects[i]->update();
        }
    }

    vector<int>::reverse_iterator idx;
    for (idx = markedForDelete.rbegin(); idx != markedForDelete.rend(); idx++)
    {
        delete *(objects.begin() + (*idx));
        objects.erase(objects.begin() + (*idx));
    }

    sort(objects.begin(), objects.end(),
        [](Object* o1, Object* o2) -> bool { return o1->getX() < o2->getX(); }
    );
}

void ObjectManager::displayObjects()
{
    vector<Object*>::iterator obj;
    for(obj = objects.begin(); obj != objects.end(); obj++)
    {
        (*obj)->display();
    }
}

void ObjectManager::init(int level)
{
    switch(level)
    {
        case 0:
            for (int i = 0; i < 4; i++)
            {
                ObjectManager::createObject(OBJECT_ASTEROID, NULL);
            }
            break;

        default:
            ObjectManager::createObject(OBJECT_SPACESHIP, NULL); // TODO: save old spaceship no replace
            for (int i = 0; i < level + 1; i++)
            {
                ObjectManager::createObject(OBJECT_ASTEROID, NULL);
            }
            break;
    }
}

void ObjectManager::addPoints(int p)
{
    points += p;
}

int ObjectManager::getPoints()
{
    int result = points;
    points = 0;
    return result;
}

void ObjectManager::removeAsteroid()
{
    nbAsteroids--;
}

int ObjectManager::getNbAsteroids()
{
    return nbAsteroids;
}

void ObjectManager::spaceshipDestroyed()
{
    spaceshipActive = false;
}

bool ObjectManager::isSpaceshipActive()
{
    return spaceshipActive;
}
