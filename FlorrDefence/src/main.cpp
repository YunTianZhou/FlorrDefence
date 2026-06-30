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
    std::cout << "--- Florr Defence ---" << std::endl;
    sf::Clock clock;
    load();
    std::cout << "Loading took " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;
    
    sf::RenderWindow window;

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 6;
    window.create(sf::VideoMode(WINDOW_INIT_SIZE), "Florr Defence", sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    while (true) {
        std::cout << "Starting game..." << std::endl;

        Game game(window);
        bool restart = game.run();

        if (!restart) break;
    }
}
