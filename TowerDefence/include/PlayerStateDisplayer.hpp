#pragma once
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "RoundRect.hpp"

class PlayerStateDisplayer : public sf::Drawable {
public:
	PlayerStateDisplayer(const PlayerState& state);

	void update();

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	const PlayerState& m_state;

	// Hp bar
	sf::RoundRect m_hpBarBackground;
	sf::RoundRect m_hpBarFill;
	sf::Text m_hpText;

	// Xp bar
	sf::RoundRect m_xpBarBackground;
	sf::RoundRect m_xpBarFill;
	sf::Text m_levelText;

	// Shield bar
	sf::RoundRect m_shieldBar;

	// Coin
	sf::RoundRect m_coinShadowRect;
	sf::RectangleShape m_coinRect;
	sf::Text m_coinText;

	// Talent
	sf::RoundRect m_talentShadowRect;
	sf::RectangleShape m_talentRect;
	sf::Text m_talentText;
};
