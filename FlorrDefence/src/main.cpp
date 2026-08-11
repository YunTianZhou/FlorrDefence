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
    if (SHOW_CONSOLE) {
        // Ensure a console is present and visible
        HWND console = GetConsoleWindow();
        if (!console) {
            if (AllocConsole()) {
                FILE* fDummy;
                freopen_s(&fDummy, "CONOUT$", "w", stdout);
                freopen_s(&fDummy, "CONIN$", "r", stdin);
                freopen_s(&fDummy, "CONOUT$", "w", stderr);
            }
            console = GetConsoleWindow();
        }
        if (console) ShowWindow(console, SW_SHOW);
    }
    else {
        // Hide the console
        HWND console = GetConsoleWindow();
        if (console) ShowWindow(console, SW_HIDE);
        FreeConsole();
    }
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
