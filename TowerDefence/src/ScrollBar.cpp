#include "ScrollBar.hpp"

ScrollBar::ScrollBar() {
    m_bar.setFillColor(sf::Color::White);
}

void ScrollBar::setSize(sf::Vector2f size) {
    m_size = size;
    updateThumb();
}

void ScrollBar::setFillColor(const sf::Color& color) {
    m_bar.setFillColor(color);
}

void ScrollBar::setContentHeight(float height) {
    m_contentHeight = std::max(height, m_viewHeight);
    clamp();
    updateThumb();
}

void ScrollBar::setViewHeight(float height) {
    m_viewHeight = height;
    m_contentHeight = std::max(m_contentHeight, m_viewHeight);
    clamp();
    updateThumb();
}

void ScrollBar::setOffset(float offset) {
    m_offset = offset;
    clamp();
    updateThumb();
}

void ScrollBar::update(sf::Vector2f mousePos) {
    m_mousePos = mousePos;

    if (!m_dragging)
        return;

    sf::Vector2f local = getInverseTransform().transformPoint(mousePos);
    float y = local.y - m_dragAnchor;
    float thumbH = m_bar.getSize().y;
    float range = m_size.y - thumbH;
    float t = (range > 0.f) ? (y / range) : 0.f;
    setOffset(t * (m_contentHeight - m_viewHeight));
}

void ScrollBar::onMouseButtonPressed(const sf::Event::MouseButtonPressed& event) {
    if (event.button != sf::Mouse::Button::Left) return;
    sf::Vector2f local = getInverseTransform().transformPoint(m_mousePos);
    if (sf::FloatRect(m_bar.getPosition(), m_bar.getSize()).contains(local)) {
        m_dragging = true;
        m_dragAnchor = local.y - m_bar.getPosition().y;
    }
}

void ScrollBar::onMouseButtonReleased(const sf::Event::MouseButtonReleased& event) {
    if (event.button != sf::Mouse::Button::Left) return;
    m_dragging = false;
}

void ScrollBar::onMouseWheelScrolled(const sf::Event::MouseWheelScrolled& event) {
    setOffset(m_offset - event.delta * 30.f);
}

void ScrollBar::clamp() {
    float maxOffset = std::max(0.f, m_contentHeight - m_viewHeight);
    if (m_offset < 0.f) m_offset = 0.f;
    if (m_offset > maxOffset) m_offset = maxOffset;
}

void ScrollBar::updateThumb() {
    float thumbH = (m_viewHeight / m_contentHeight) * m_size.y;
    thumbH = std::clamp(thumbH, 10.f, m_size.y);
    m_bar.setSize({ m_size.x, thumbH });
    m_bar.setRadius(m_size.x / 2.f);

    float range = m_size.y - thumbH;
    float t = (m_contentHeight > m_viewHeight)
        ? (m_offset / (m_contentHeight - m_viewHeight))
        : 0.f;
    m_bar.setPosition({ 0.f, t * range });
}

void ScrollBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(m_bar, states);
}
