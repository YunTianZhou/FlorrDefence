#pragma once
#include <SFML/Graphics.hpp>
#include "RoundRect.hpp"

class ScrollBar : public sf::Drawable, public sf::Transformable {
public:
    ScrollBar();

    void setSize(sf::Vector2f size);
    void setFillColor(const sf::Color& color);
    void setContentHeight(float height);
    void setViewHeight(float height);
    void setOffset(float offset);

    sf::Vector2f getSize() const { return m_size; }
    float getContentHeight() const { return m_contentHeight; }
    float getViewHeight() const { return m_viewHeight; }
    float getOffset() const { return m_offset; }

    void update(sf::Vector2f mousePos);
    void onMouseButtonPressed(const sf::Event::MouseButtonPressed& event);
    void onMouseButtonReleased(const sf::Event::MouseButtonReleased& event);
    void onMouseWheelScrolled(const sf::Event::MouseWheelScrolled& event);

private:
    void clamp();
    void updateThumb();
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    sf::Vector2f    m_size;
    float           m_contentHeight = 0.f;
    float           m_viewHeight = 0.f;
    float           m_offset = 0.f;

    sf::RoundRect   m_bar;
    bool            m_dragging = false;
    float           m_dragAnchor = 0.f;
    sf::Vector2f    m_mousePos;
};
