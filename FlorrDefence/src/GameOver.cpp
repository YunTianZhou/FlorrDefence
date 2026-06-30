#include "GameOver.hpp"

#include "AssetManager.hpp"
#include "Constants.hpp"
#include "Tools.hpp"

GameOver::GameOver()
    : m_titleText(AssetManager::getFont()),
      m_targetText(AssetManager::getFont()) {

    // Shade
    m_shade.setPosition({ 0.f, 0.f });
    m_shade.setSize(VIEW_SIZE);
    m_shade.setFillColor(sf::Color(0, 0, 0, 128));

    float centerX = VIEW_SIZE.x / 2.f;
    float startY = 420.f;

    // Title
    m_titleText.setPosition({ centerX, startY });
    m_titleText.setString("You were destroyed by:");
    m_titleText.setCharacterSize(20);
    m_titleText.setFillColor(sf::Color::White);
    m_titleText.setOutlineColor(sf::Color::Black);
    m_titleText.setOutlineThickness(1.f);
    m_titleText.setOrigin(m_titleText.getGlobalBounds().size / 2.f + m_titleText.getLocalBounds().position);

    // Target
    m_targetText.setPosition({ centerX, startY + 35.f });
    m_targetText.setCharacterSize(32);
    m_targetText.setFillColor(sf::Color::White);
    m_targetText.setOutlineColor(sf::Color::Black);
    m_targetText.setOutlineThickness(2.f);

    // Icon (dead)
    m_icon.setPosition({ centerX, startY + 120.f });
    m_icon.setSize({ 80.f, 80.f });
    m_icon.setOrigin(m_icon.getSize() / 2.f);
    m_icon.setTexture(&AssetManager::getUITexture("dead"));

    // Continue button
    m_continueButton.setSize({ 150.f, 45.f });
    m_continueButton.setString("Continue");
    m_continueButton.setCharactorSize(30);
    m_continueButton.setFillColor(sf::Color(29, 209, 41));
    m_continueButton.setOutline(sf::Color(23, 169, 33), 8.f);
    m_continueButton.setPosition(sf::Vector2f(centerX, startY + 220.f) - (m_continueButton.getRect().size / 2.f));
}

void GameOver::setTarget(const std::string& target) {
    if (m_target == target)
        return;

    m_target = target;
    m_targetText.setString(capitalized(target));
    m_targetText.setOrigin(m_targetText.getGlobalBounds().size / 2.f + m_targetText.getLocalBounds().position);
}

void GameOver::update(sf::Vector2f mousePos) {
    m_continueButton.update(mousePos);
}

void GameOver::onEvent(const sf::Event& event) {
    if (auto pressed = event.getIf<sf::Event::MouseButtonPressed>())
        m_continueButton.onMouseButtonPressed(*pressed);
    else if (auto released = event.getIf<sf::Event::MouseButtonReleased>())
        if (m_continueButton.onMouseButtonReleased(*released))
            m_readyToContinue = true;
}

bool GameOver::readyToContinue() const {
    return m_readyToContinue;
}

void GameOver::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_shade, states);
    target.draw(m_titleText, states);
    target.draw(m_targetText, states);
    target.draw(m_icon, states);
    target.draw(m_continueButton, states);
}
