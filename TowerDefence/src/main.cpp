#include <iostream>
#include "Game.hpp"
#include "AssetManager.hpp"
#include "SpriteCollisionManager.hpp"
#include "Constants.hpp"

void load() {
    loadConstants();
    AssetManager::load();
    SpriteCollisionManager::load();
}

int main() {
    std::cout << "--- Tower Defence ---" << std::endl;
    sf::Clock clock;
    load();
    std::cout << "Loading took " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;

    Game game;
    game.run();
}
