#pragma once
#include <SFML/Graphics.hpp>

class ViewManager {
public: 
    ViewManager(const sf::Vector2f& viewSize);

    void onResize(sf::Vector2u windowSize);
    const sf::View& getView() const;

private:
    sf::View m_view;
    sf::Vector2f m_baseSize;
};
