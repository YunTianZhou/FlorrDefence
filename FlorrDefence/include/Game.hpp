#pragma once
#include <SFML/Graphics.hpp>
#include "ViewManager.hpp"
#include "SharedInfo.hpp"
#include "Map.hpp"
#include "UI.hpp"
#include "Record.hpp"
#include "GameOver.hpp"

class Game {
public:
    Game(sf::RenderWindow& window);
    bool run();

private:
    void handleEvents();
    void handleSpecialKey(sf::Keyboard::Key keyCode);
    void update();
    void render();

    bool trySaveToPath(const std::filesystem::path& path);

private:
    sf::RenderWindow& m_window;
    int m_frameCount = 0;
    float m_elapsedTime = 0.f;
    sf::Clock m_saveCooldownClock;
    bool m_hasSavedOnce = false;

    ViewManager m_viewManager;
    SharedInfo m_info;
    Map m_map;
    UI m_ui;
    Record m_record;
    GameOver m_gameOver;
};
