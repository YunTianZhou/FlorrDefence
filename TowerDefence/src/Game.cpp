#include <iostream>
#include "Game.hpp"
#include "Constants.hpp"

Game::Game()
    : m_viewManager(VIEW_SIZE), m_map(m_info), m_ui(m_info), 
      m_record(m_info, m_map, m_ui.m_shop, m_ui.m_talent) {
    m_record.try_load("TowerDefence.json");

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 6;
    m_window.create(sf::VideoMode(WINDOW_INIT_SIZE), "Tower Defence", sf::Style::Default, sf::State::Windowed, settings);
    m_window.setView(m_viewManager.getView());
    m_window.setFramerateLimit(60); 
}

void Game::run() {
    while (m_window.isOpen()) {
        handleEvents();
        update();
        render();

        m_elapsedTime += m_info.dt.asSeconds();
        m_frameCount++;

        if (m_elapsedTime >= 1.f) {
            std::cout << "FPS: " << m_frameCount << std::endl;
            m_frameCount = 0;
            m_elapsedTime = 0.f;
        }
    }

    m_record.save("TowerDefence.json");
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
            if (const auto* keyEvent = event->getIf<sf::Event::KeyReleased>()) {
                switch (keyEvent->code) {
                case sf::Keyboard::Key::S:
                    if(m_info.input.keyCtrl)
                        m_record.save("TowerDefence.json");
                    break;
                // TEST
                case sf::Keyboard::Key::W:
                    m_info.playerState.level++;
                    break;
                case sf::Keyboard::Key::Q:
                    m_info.playerState.level += 10;
                    break;
                case sf::Keyboard::Key::E:
                    m_info.playerState.level--;
                    break;
                case sf::Keyboard::Key::R:
                    m_info.playerState.level -= 10;
                    break;
                case sf::Keyboard::Key::T:
                    m_info.playerState.level = 1;
                    break;
                }
            }

            if (m_map.onEvent(*event))
                m_ui.updateComponents();
            m_ui.onEvent(*event);

            // NOTE: This must run after map.onEvent()
            if (const auto* releasedEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (releasedEvent->button == sf::Mouse::Button::Left && m_info.draggedCard.has_value())
                    m_info.draggedCard->startRetreat();
            }
        }
    }
}

void Game::update() {
    if (m_info.update(m_window))
        m_ui.updateComponents();
    if (m_map.update())
        m_ui.updateComponents();
    m_ui.update();
}

void Game::render() {
    m_window.clear();

    m_window.draw(m_map);
    m_window.draw(m_ui);
    if (m_info.draggedCard.has_value())
        m_window.draw(*m_info.draggedCard);
    if (m_info.cardDescription.isVerified())
        m_window.draw(m_info.cardDescription);

    m_window.display();
}

