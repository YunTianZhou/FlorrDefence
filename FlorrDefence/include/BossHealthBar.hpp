#pragma once
#include <SFML/Graphics.hpp>
#include "RoundRect.hpp"
#include "Constants.hpp"

class BossHealthBar : public sf::Drawable {
public:
    BossHealthBar();

    void update(const MobInfo& mob, float hp);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    MobInfo m_mob;
    float m_hpRatio = 0.f;

    sf::RoundRect m_hpBarBackground;
    sf::RoundRect m_hpBarFill;

    sf::Text m_typeText;
    sf::Text m_rarityText;
};
