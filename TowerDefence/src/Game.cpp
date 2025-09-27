#include <iostream>
#include "Game.hpp"
#include "Constants.hpp"
#include "AssetManager.hpp"
#include "SpriteCollisionManager.hpp"

Game::Game()
    : m_viewManager(VIEW_SIZE), m_map(m_info), m_ui(m_info) {
    m_info.init();

    m_window.create(sf::VideoMode(WINDOW_INIT_SIZE), "Tower Defence");
    m_window.setView(m_viewManager.getView());
    m_window.setFramerateLimit(60);
}

void Game::run() {
    m_fpsClock.restart();

    while (m_window.isOpen()) {
        handleEvents();
        update();
        render();

        float dt = m_fpsClock.restart().asSeconds();
        m_elapsedTime += dt;
        m_frameCount++;

        if (m_elapsedTime >= 1.f) {
            std::cout << "FPS: " << m_frameCount << std::endl;
            m_frameCount = 0;
            m_elapsedTime = 0.f;
        }
    }
}

void Game::handleEvents() {
    while (std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        }
        else if (const auto* resizeEvent = event->getIf<sf::Event::Resized>()) {
            m_viewManager.onResize(resizeEvent->size);
            m_window.setView(m_viewManager.getView());
        }
        else {
            // TEST
            if (const auto* keyEvent = event->getIf<sf::Event::KeyReleased>()) {
                if (keyEvent->code == sf::Keyboard::Key::W) {
                    m_info.playerState.level++;
                }
            }
            m_map.onEvent(*event);
            m_ui.onEvent(*event);
            if (const auto* releasedEvent = event->getIf<sf::Event::MouseButtonReleased>())
                if (releasedEvent->button == sf::Mouse::Button::Left && m_info.draggedCard.has_value())
                    m_info.draggedCard->startRetreat();
        }
    }
}

void Game::update() {
    if (m_info.update(m_window))
        m_ui.updateComponents();
    m_map.update();
    m_ui.update();
}

void Game::render() {
    m_window.clear();

    m_window.draw(m_map);
    m_window.draw(m_ui);
    if (m_info.draggedCard.has_value())
        m_window.draw(*m_info.draggedCard);

    m_window.display();
}

