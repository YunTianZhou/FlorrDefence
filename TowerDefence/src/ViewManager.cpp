#include "ViewManager.hpp"

ViewManager::ViewManager(const sf::Vector2f& viewSize)
    : m_baseSize(viewSize), m_view(sf::FloatRect({ 0.f, 0.f }, viewSize)) {

}

void ViewManager::onResize(sf::Vector2u windowSize) {
    float windowRatio = float(windowSize.x) / windowSize.y;
    float viewRatio = m_baseSize.x / m_baseSize.y;

    sf::Vector2f size(1.f, 1.f);
    sf::Vector2f pos(0.f, 0.f);

    if (windowRatio > viewRatio) {
        size.x = viewRatio / windowRatio;
        pos.x = (1.f - size.x) / 2.f;
    }
    else {
        size.y = windowRatio / viewRatio;
        pos.y = (1.f - size.y) / 2.f;
    }

    m_view.setViewport(sf::FloatRect(pos, size));
}

const sf::View& ViewManager::getView() const {
    return m_view;
}
