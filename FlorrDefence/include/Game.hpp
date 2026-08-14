#pragma once
#include <SFML/Graphics.hpp>
#include "ViewManager.hpp"
#include "SharedInfo.hpp"
#include "Map.hpp"
#include "UI.hpp"
#include "GameOver.hpp"

class Game {
public:
    enum class Request {
        None,
        Restart,
        Load,
        Quit
    };

public:
    Game(sf::RenderWindow& window);
    void start();
    void run();

    Request popRequest();
    std::filesystem::path getRequestPath() const { return m_requestPath; }

    friend class Record;

private:
    void handleEvents();
    void handleSpecialKey(sf::Keyboard::Key keyCode);
    void update();
    void render();

    void handleFileDialog();
    bool trySaveToPath(const std::filesystem::path& path, bool ignoreThreshold = false);

private:
    sf::RenderWindow* m_window;
    int m_frameCount = 0;
    float m_elapsedTime = 0.f;
    sf::Clock m_saveCooldownClock;
    sf::Clock autoSaveClock;
    bool m_hasSavedOnce = false;

    Request m_request = Request::None;
    std::filesystem::path m_requestPath;

    ViewManager m_viewManager;
    SharedInfo m_info;
    Map m_map;
    UI m_ui;
    GameOver m_gameOver;
};
