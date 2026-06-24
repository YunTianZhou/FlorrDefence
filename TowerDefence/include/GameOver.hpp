#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include "Button.hpp"

class GameOver : public sf::Drawable {
public:
    GameOver();

    void setTarget(const std::string& target);

    void update(sf::Vector2f mousePos);
    void onEvent(const sf::Event& event);

    bool readyToContinue() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    std::string m_target;
    bool m_readyToContinue = false;

    sf::RectangleShape m_shade;
    sf::RectangleShape m_icon;

    sf::Text m_titleText;
    sf::Text m_targetText;

    ClickButton m_continueButton;
};
