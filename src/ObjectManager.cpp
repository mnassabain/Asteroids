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
    vector<int> markedForDelete;

    int size = objects.size();
    vector<Object*>::iterator end = objects.end();
    vector<Object*>::iterator obj;
    int index = 0;
    for(obj = objects.begin(); obj != end; obj++)
    {
        if ((*obj)->isDestroyed())
        {
            markedForDelete.push_back(index);
        }
        else
        {
            // check collisions
            int counter = 1;
            while(counter < size - index)
            {
                if (!(*(obj + counter))->isDestroyed())
                    (*obj)->collidingWith(*(obj + counter));
                counter++;
            }

            // update
            (*obj)->update();
        }
        index++;
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
