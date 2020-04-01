#include <EventManager.hpp>

int EventManager::handleEvents()
{
    int result = 1;
    
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                result = EVT_CLOSE_GAME;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_z:
                        events.push(EVT_PLAYER_ACCELERATE);
                        break;

                    case SDLK_q:
                        events.push(EVT_PLAYER_START_TURN_CCW);
                        break;

                    case SDLK_d:
                        events.push(EVT_PLAYER_START_TURN_CW);
                        break;
                }
                break;

            case SDL_KEYUP:
                switch(event.key.keysym.sym)
                {
                    case SDLK_z:
                        events.push(EVT_PLAYER_DECELERATE);
                        break;

                    case SDLK_q:
                    case SDLK_d:
                        events.push(EVT_PLAYER_STOP_TURN);
                        break;
                }
                break;
        }
    }

    return result;
}

int EventManager::poll()
{
    Event event;
    if (events.empty())
        event = NO_EVENTS;
    else
    {
        event = events.front();
        events.pop();
    }

    return event;
}