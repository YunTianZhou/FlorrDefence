#pragma once
#include <SFML/Graphics.hpp>
#include "ViewManager.hpp"
#include "SharedInfo.hpp"
#include "Map.hpp"
#include "UI.hpp"
#include "Record.hpp"

class Game {
public:
    Game();
    void run();

private:
    void handleEvents();
    void update();
    void render();

private:
    sf::RenderWindow m_window;
    sf::Clock m_fpsClock;
    int m_frameCount = 0;
    float m_elapsedTime = 0.f;

    ViewManager m_viewManager;
    SharedInfo m_info;
    Map m_map;
    UI m_ui;
    Record m_record;
};
