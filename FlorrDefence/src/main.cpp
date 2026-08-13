#include <iostream>
#include "Game.hpp"
#include "AssetManager.hpp"
#include "SpriteCollisionManager.hpp"
#include "Constants.hpp"
#include "OS.hpp"

void load() {
    loadConstants();
    AssetManager::load();
    SpriteCollisionManager::load();

#ifdef _WIN32
    OS::showConsole(SHOW_CONSOLE);
#endif
}

int main() {
    std::cout << "--- Florr Defence ---" << std::endl;
    sf::Clock clock;
    load();
    std::cout << "Loading took " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 6;

    sf::RenderWindow window;
    window.create(sf::VideoMode(WINDOW_INIT_SIZE), "Florr Defence", sf::Style::Default, sf::State::Windowed, settings);
    window.setIcon(AssetManager::getTexture("icon").copyToImage());
    if (VSYNC_ENABLED)
        window.setVerticalSyncEnabled(true);
    else
        window.setFramerateLimit(60);

    while (true) {
        std::cout << "Starting game..." << std::endl;

        Game game(window);
        bool restart = game.run();

        if (!restart) break;
    }
}
