#include <AsteroidSpriteComponent.hpp>
#include <Asteroid.hpp>

AsteroidSpriteComponent::AsteroidSpriteComponent(int size)
{
    texture = Engine::getTexture(GraphicsManager::TEXTURE_ASTEROID);
    switch (size)
    {
        case LARGE_ASTEROID:
            box = Rect(0, 0, 20, 20);
            break;
    
        case MEDIUM_ASTEROID:
            box = Rect(20, 0, 20, 20);
            break;

        case SMALL_ASTEROID:
            box = Rect(40, 0, 20, 20);
            break;
    }
}
