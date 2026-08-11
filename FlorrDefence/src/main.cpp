#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "Game.hpp"
#include "AssetManager.hpp"
#include "SpriteCollisionManager.hpp"
#include "Constants.hpp"

void load() {
    loadConstants();
    AssetManager::load();
    SpriteCollisionManager::load();

#ifdef _WIN32
    if (!DEBUG_MODE) {
        HWND console = GetConsoleWindow();
        if (console) ShowWindow(console, SW_HIDE);
        // Detach the console to ensure it does not appear in some run configurations
        FreeConsole();
    }
#endif
}

int main() {
    sf::Clock clock;
    load();
    std::cout << "Loading took " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;
    
    sf::RenderWindow window;

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 6;
    window.create(sf::VideoMode(WINDOW_INIT_SIZE), "Florr Defence", sf::Style::Default, sf::State::Windowed, settings);
    window.setIcon(AssetManager::getTexture("icon").copyToImage());
    window.setFramerateLimit(60);

    while (true) {
        std::cout << "Starting game..." << std::endl;

        Game game(window);
        bool restart = game.run();

        if (!restart) break;
    }
}
