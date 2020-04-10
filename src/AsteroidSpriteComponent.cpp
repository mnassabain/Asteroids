#include <AsteroidSpriteComponent.hpp>

AsteroidSpriteComponent::AsteroidSpriteComponent()
{
    texture = Engine::getTexture(GraphicsManager::TEXTURE_ASTEROID);
}
