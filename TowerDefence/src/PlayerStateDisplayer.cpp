#include "PlayerStateDisplayer.hpp"
#include "AssetManager.hpp"
#include "Tools.hpp"

const sf::Vector2f HP_BORDER_OFFSET = { 5.f, 5.f };
const sf::Vector2f XP_BORDER_OFFSET = { 3.f, 3.f };
const sf::Vector2f SHIELD_BORDER_OFFSET = { 5.f, 7.f };
const sf::Vector2f COIN_SYMBOL_OFFSET = { -20.f, -20.f };

PlayerStateDisplayer::PlayerStateDisplayer(const PlayerState& state)
	: m_state(state),
	m_hpText(AssetManager::getFont()),
	m_levelText(AssetManager::getFont()),
	m_coinText(AssetManager::getFont()),
	m_talentText(AssetManager::getFont()) {

	sf::Vector2f barStart = { 1100.f, 50.f };

	// Hp bar
	m_hpBarBackground.setPosition(barStart);
	m_hpBarBackground.setSize({ 300.f, 50.f });
	m_hpBarBackground.setRadius(50.f / 2.f);
	m_hpBarBackground.setFillColor(sf::Color::Black);

	m_hpBarFill.setPosition(barStart + HP_BORDER_OFFSET);
	m_hpBarFill.setSize(m_hpBarBackground.getSize() - 2.f * HP_BORDER_OFFSET);
	m_hpBarFill.setRadius(m_hpBarFill.getSize().y / 2.f);
		
	m_shieldBar.setPosition(m_hpBarFill.getPosition() + SHIELD_BORDER_OFFSET);
	m_shieldBar.setSize(m_hpBarFill.getSize() - 2.f * SHIELD_BORDER_OFFSET);
	m_shieldBar.setRadius(m_shieldBar.getSize().y / 2.f);

	m_hpText.setPosition(barStart + m_hpBarBackground.getSize() / 2.f);
	m_hpText.setFillColor(sf::Color::White);
	m_hpText.setOutlineColor(sf::Color::Black);
	m_hpText.setOutlineThickness(1.5f);
	m_hpText.setCharacterSize(30);

	// Xp bar
	m_xpBarBackground.setPosition({ barStart.x + 20.f, barStart.y + 60.f });
	m_xpBarBackground.setSize({ 260.f, 30.f });
	m_xpBarBackground.setRadius(30.f / 2.f);
	m_xpBarBackground.setFillColor(sf::Color::Black);

	m_xpBarFill.setPosition(m_xpBarBackground.getPosition() + XP_BORDER_OFFSET);
	m_xpBarFill.setSize(m_xpBarBackground.getSize() - 2.f * XP_BORDER_OFFSET);
	m_xpBarFill.setRadius(m_xpBarFill.getSize().y / 2.f);

	m_levelText.setPosition(m_xpBarBackground.getPosition() + m_xpBarBackground.getSize() / 2.f);
	m_levelText.setFillColor(sf::Color::White);
	m_levelText.setOutlineColor(sf::Color::Black);
	m_levelText.setOutlineThickness(1.f);
	m_levelText.setCharacterSize(20);

	sf::Vector2f labelStart = { 1475.f, 40.f };

	// Coin
	m_coinShadowRect.setPosition(labelStart);
	m_coinShadowRect.setSize({ 50.f, 50.f });
	m_coinShadowRect.setRadius(5.f);
	m_coinShadowRect.setFillColor(DARK_COLORS.at("wood"));

	m_coinRect.setPosition(labelStart + COIN_SYMBOL_OFFSET);
	m_coinRect.setSize(sf::Vector2f(50.f, 50.f) - 2.f * COIN_SYMBOL_OFFSET);
	m_coinRect.setTexture(&AssetManager::getUITexture("coin"));

	m_coinText.setString("0");
	m_coinText.setOrigin({ 0.f, m_coinText.getGlobalBounds().size.y / 2.f +
								m_coinText.getLocalBounds().position.y });
	m_coinText.setPosition({ labelStart.x + 50.f, labelStart.y + 25.f });
	m_coinText.setFillColor(sf::Color::White);
	m_coinText.setOutlineColor(sf::Color::Black);
	m_coinText.setOutlineThickness(1.5f);
	m_coinText.setCharacterSize(30);

	// Talent
	m_talentShadowRect = m_coinShadowRect;
	m_talentShadowRect.move({ 0.f, 60.f });

	m_talentRect = m_coinRect;
	m_talentRect.move({ 0.f, 60.f });
	m_talentRect.setTexture(&AssetManager::getUITexture("talent"));

	m_talentText = m_coinText;
	m_talentText.move({ 0.f, 60.f });
}

void PlayerStateDisplayer::update() {
	auto updateBar = [](sf::Vector2f size, float ratio, sf::RoundRect& barFill, 
						sf::Color full, sf::Color zero) {
		float        diameter = 2.f * barFill.getRadius();
		float        rawWidth = ratio * size.x;
		float        width = std::max(diameter, rawWidth);

		barFill.setSize({ width, barFill.getSize().y });

		if (rawWidth < diameter) {
			float t = 1.f - (rawWidth / diameter);
			unsigned char R = (unsigned char)(full.r - t * zero.r);
			unsigned char G = (unsigned char)(full.g - t * zero.g);
			unsigned char B = (unsigned char)(full.b - t * zero.b);
			unsigned char A = (unsigned char)(full.a - t * zero.a);
			barFill.setFillColor({ R, G, B, A });
		}
		else {
			barFill.setFillColor(full);
		}
	};

	// Hp bar
	float hpRatio = std::clamp(float(m_state.hp) / m_state.hpLimit, 0.f, 1.f);
	sf::Vector2f hpSize = m_hpBarBackground.getSize() - 2.f * HP_BORDER_OFFSET;
	updateBar(hpSize, hpRatio, m_hpBarFill, { 105, 210, 57 }, { 50, 100, 30 });
	m_hpText.setString(toNiceString(m_state.hp) + "/" + toNiceString(m_state.hpLimit));
	m_hpText.setOrigin(m_hpText.getGlobalBounds().size / 2.f + m_hpText.getLocalBounds().position);

	// Xp bar
	float xpRatio = std::clamp(float(m_state.xp) / m_state.calcRequiredXp(), 0.f, 1.f);
	sf::Vector2f xpSize = m_xpBarBackground.getSize() - 2.f * XP_BORDER_OFFSET;
	updateBar(xpSize, xpRatio, m_xpBarFill, { 213, 241, 100 }, { 100, 120, 50 });
	m_levelText.setString("Lvl " + std::to_string(m_state.level));
	m_levelText.setOrigin(m_levelText.getGlobalBounds().size / 2.f + m_levelText.getLocalBounds().position);

	// Shield bar
	float shieldRatio = std::clamp(float(m_state.shield) / m_state.hpLimit, 0.0f, 1.f);
	sf::Vector2f shieldSize = hpSize - 2.f * SHIELD_BORDER_OFFSET;
	updateBar(shieldSize, shieldRatio, m_shieldBar, { 240, 234, 240 }, { 0, 0, 0, 240 });

	// Coin
	m_coinText.setString(toNiceString(m_state.coin));
	float textRight = m_coinText.getPosition().x + m_coinText.getGlobalBounds().size.x;
	m_coinShadowRect.setSize({ textRight - m_coinShadowRect.getPosition().x + 10.f, 50.f});

	// Talent
	m_talentText.setString(std::to_string(m_state.talent));
	textRight = m_talentText.getPosition().x + m_talentText.getGlobalBounds().size.x;
	m_talentShadowRect.setSize({ textRight - m_talentShadowRect.getPosition().x + 10.f, 50.f });
}

void PlayerStateDisplayer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// Hp bar
	target.draw(m_hpBarBackground, states);
	if (m_state.hp > 0)
		target.draw(m_hpBarFill, states);
	if (m_state.shield > 0)
		target.draw(m_shieldBar, states);
	target.draw(m_hpText, states);

	// Xp bar
	target.draw(m_xpBarBackground, states);
	if (m_state.xp)
		target.draw(m_xpBarFill, states);
	target.draw(m_levelText, states);

	// Coin
	target.draw(m_coinShadowRect, states);
	target.draw(m_coinRect, states);
	target.draw(m_coinText, states);

	// Talent
	target.draw(m_talentShadowRect, states);
	target.draw(m_talentRect, states);
	target.draw(m_talentText, states);
}
