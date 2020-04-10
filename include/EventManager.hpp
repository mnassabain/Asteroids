#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include <queue>
#include <SDL2/SDL.h>

#include <EventCodes.hpp>

using namespace std;

typedef int Event;

class EventManager
{
    private:
        queue<Event> events;

    public:
        int handleEvents();
        void clearEvents();
        int poll();
};


#endif /* EVENTMANAGER_HPP */
