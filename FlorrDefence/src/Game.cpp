#include <iostream>
#include <filesystem>

#include "Game.hpp"
#include "Constants.hpp"
#include "OS.hpp"
#include "Record.hpp"

Game::Game(sf::RenderWindow& window)
    : m_window(&window),
      m_viewManager(VIEW_SIZE),
      m_map(&m_info),
      m_ui(&m_info) {}

void Game::start() {
    m_info.dtClock.restart();
    m_map.getMapInfo().tick();  // init buff to prevent problems

    // Initialize save cooldown so first save is allowed immediately
    m_saveCooldownClock.restart();
    autoSaveClock.restart();
    m_hasSavedOnce = false;

    // Window view
    m_viewManager.onResize(m_window->getSize());
    m_window->setView(m_viewManager.getView());
}

void Game::run() {
    if (!m_window->isOpen()) {
        m_request = Request::Quit;
        return;
    }

    handleEvents();
    update();
    render();

    if (!m_info.playerState.isAlive() && m_gameOver.readyToContinue()) {
        m_request = Request::Restart;
        return;
    }

    m_elapsedTime += m_info.dt.asSeconds();
    m_frameCount++;

    if (m_elapsedTime >= 1.f) {
        if (DEBUG_MODE)
            std::cout << "FPS: " << m_frameCount << std::endl;

        m_frameCount = 0;
        m_elapsedTime = 0.f;
    }

    // Auto-save
    if (AUTO_SAVE_ENABLED && m_info.playerState.isAlive()) {
        if (autoSaveClock.getElapsedTime().asSeconds() >=
            static_cast<float>(AUTO_SAVE_INTERVAL_SECONDS)) {

            std::cout << "Auto saving..." << std::endl;

            trySaveToPath(std::filesystem::path(SAVE_PATH_DEFAULT));
            autoSaveClock.restart();
        }
    }
}

Game::Request Game::popRequest() {
    Request r = m_request;
    m_request = Request::None; 
    return r;
}

void Game::handleEvents() {
    while (std::optional event = m_window->pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window->close();
        }
        else if (const auto* resizeEvent = event->getIf<sf::Event::Resized>()) {
            m_viewManager.onResize(resizeEvent->size);
            m_window->setView(m_viewManager.getView());
        }
        else {
            if (const auto* keyEvent = event->getIf<sf::Event::KeyReleased>()) {
                handleSpecialKey(keyEvent->code);
            }

            if (m_info.playerState.isAlive()) {
                if (m_map.onEvent(*event))
                    m_ui.updateComponents();

                m_ui.onEvent(*event);

                // NOTE: This must run after map.onEvent()
                if (const auto* releasedEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (releasedEvent->button == sf::Mouse::Button::Left &&
                        m_info.draggedCard.has_value()) {

                        m_info.draggedCard->startRetreat();
                    }
                }
            }
            else {
                m_gameOver.onEvent(*event);
            }
        }
    }
}

void Game::update() {
    handleFileDialog();

    bool needUpdate = m_info.update(*m_window);

    if (m_info.playerState.isAlive()) {
        if (needUpdate)
            m_ui.updateComponents();

        if (m_map.update())
            m_ui.updateComponents();

        m_ui.update();
    }
    else {
        m_gameOver.update(m_info.mouseWorldPosition);
    }
}

void Game::render() {
    m_window->clear();

    m_window->draw(m_map);
    m_window->draw(m_ui);

    if (m_info.draggedCard.has_value())
        m_window->draw(*m_info.draggedCard);

    if (m_info.cardDescription.isVerified())
        m_window->draw(m_info.cardDescription);

    if (!m_info.playerState.isAlive()) {
        m_gameOver.setTarget(m_info.playerState.lastHitMob.type);
        m_window->draw(m_gameOver);
    }

    m_window->display();
}

void Game::handleFileDialog() {
    auto result = OS::pollDialog();
    if (!result)
        return;

    std::filesystem::path path(result->path);

    if (result->type == OS::DialogType::Save) {
        trySaveToPath(path, true);
        return;
    }
    else if (result->type == OS::DialogType::Open) {
        if (!std::filesystem::exists(path)) {
            std::cout << "Selected file does not exist: " << path.string() << std::endl;
            return;
        }

        m_request = Request::Load;
        m_requestPath = path;
    }
}

bool Game::trySaveToPath(const std::filesystem::path& path, bool ignoreThreshold) {
    try {
        if (path.empty()) {
            std::cerr << "Invalid save path." << std::endl;
            return false;
        }

        if (!ignoreThreshold && m_hasSavedOnce) {
            // Enforce 5 second save threshold
            float elapsed = m_saveCooldownClock.getElapsedTime().asSeconds();

            if (elapsed < 5.f) {
                std::cout << "[WARNING] Saving too fast (" << elapsed << "s). Wait at least 5s between saves." << std::endl;
                return false;
            }
        }

        // Ensure parent directory exists
        try {
            auto parent = path.parent_path();

            if (!parent.empty() && !std::filesystem::exists(parent))
                std::filesystem::create_directories(parent);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to prepare save directory: " << e.what() << std::endl;
            return false;
        }

        // Perform save
        try {
            Record::instance().save(*this, path);

            m_saveCooldownClock.restart();
            m_hasSavedOnce = true;

            std::cout << "Saved to '" << path.string() << "'" << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Save failed: " << e.what() << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Unexpected error during save: " << e.what() << std::endl;
        return false;
    }
}

void Game::handleSpecialKey(sf::Keyboard::Key keyCode) {
    if (m_info.playerState.isAlive()) {

        // Ctrl + S -> Save
        if (m_info.input.keyCtrl && !m_info.input.keyShift &&
            keyCode == sf::Keyboard::Key::S) {

            trySaveToPath(std::filesystem::path(SAVE_PATH_DEFAULT));
            return;
        }

        // Ctrl + Shift + S -> Save As
        if (m_info.input.keyCtrl && m_info.input.keyShift &&
            keyCode == sf::Keyboard::Key::S) {

            if (!OS::saveAs())
                std::cout << "A file dialog is already open." << std::endl;

            return;
        }
    }

    // Ctrl + O -> Open file
    if (m_info.input.keyCtrl && keyCode == sf::Keyboard::Key::O) {
        if (!OS::open())
            std::cout << "A file dialog is already open." << std::endl;
        return;
    }

    // Debug keys
    if (DEBUG_MODE && m_info.playerState.isAlive()) {
        switch (keyCode) {
        case sf::Keyboard::Key::Num1:
            m_info.playerState.level++;
            break;

        case sf::Keyboard::Key::Num2:
            m_info.playerState.level += 10;
            break;

        case sf::Keyboard::Key::Num3:
            m_info.playerState.level += 100;
            break;

        case sf::Keyboard::Key::Num4:
            m_info.playerState.level -= 100;
            break;

        case sf::Keyboard::Key::Q:
            m_info.playerState.coin += 10'000'000'000'000ll;
            break;

        case sf::Keyboard::Key::Num0:
            m_info.playerState.level = 1;
            break;

        default:
            break;
        }
    }
}
