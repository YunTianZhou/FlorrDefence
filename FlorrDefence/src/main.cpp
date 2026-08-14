#include <iostream>
#include "Game.hpp"
#include "AssetManager.hpp"
#include "SpriteCollisionManager.hpp"
#include "Constants.hpp"
#include "OS.hpp"
#include "Record.hpp"

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

    // Init window
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 6;

    sf::RenderWindow window;
    window.create(sf::VideoMode(WINDOW_INIT_SIZE), "Florr Defence", sf::Style::Default, sf::State::Windowed, settings);
    window.setIcon(AssetManager::getTexture("icon").copyToImage());
    if (VSYNC_ENABLED)
        window.setVerticalSyncEnabled(true);
    else
        window.setFramerateLimit(60);

    // Game
    Record& record = Record::instance();

    auto game = std::make_unique<Game>(window);
    if (!record.try_load(*game, LOAD_PATH_DEFAULT)) {
        std::cerr << "Invalid record, program terminates." << std::endl;
        return -1;
    }
    game->start();

    while (window.isOpen()) {
        game->run();

        switch (game->popRequest()) {
        case Game::Request::None:
            break;

        case Game::Request::Quit:
            return 0;

        case Game::Request::Restart: {
            auto new_game = std::make_unique<Game>(window);
            if (record.try_load(*new_game, LOAD_PATH_DEFAULT)) {
                game = std::move(new_game);
                game->start();
            }
            break;
        }

        case Game::Request::Load: {
            auto new_game = std::make_unique<Game>(window);
            if (record.try_load(*new_game, game->getRequestPath())) {
                game = std::move(new_game);
                game->start();
            }
            break;
        }
        }
    }
}
