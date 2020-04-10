#include <SpaceshipSpriteComponent.hpp>

SpaceshipSpriteComponent::SpaceshipSpriteComponent()
{
    texture = Engine::getTexture(GraphicsManager::TEXTURE_SPACESHIP);
}
