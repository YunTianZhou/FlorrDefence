#include "BossHealthBar.hpp"
#include "AssetManager.hpp"
#include "Tools.hpp"
#include "Constants.hpp"

const sf::Vector2f barPosition = { 550.f, 100.f };
const sf::Vector2f barSize = { 400.f, 50.f };
const sf::Vector2f borderOffset = { 7.f, 7.f };
const sf::Vector2f barFillSize = barSize - 2.f * borderOffset;
const sf::Vector2f textOffset = { 0.f, (barSize.y - borderOffset.y) / 2.f };

BossHealthBar::BossHealthBar() 
	: m_typeText(AssetManager::getFont()),
	  m_rarityText(AssetManager::getFont()) {

	// Hp bar
	m_hpBarBackground.setPosition(barPosition);
	m_hpBarBackground.setSize(barSize);
	m_hpBarBackground.setRadius(barSize.y / 2.f);
	m_hpBarBackground.setFillColor(sf::Color::Black);
	m_hpBarBackground.setOrigin(barSize / 2.f);

	m_hpBarFill.setPosition(barPosition);
	m_hpBarFill.setSize(barFillSize);
	m_hpBarFill.setRadius(m_hpBarFill.getSize().y / 2.f);
	m_hpBarFill.setOrigin(barFillSize / 2.f);

	// Type text
	m_typeText.setPosition(barPosition - textOffset);
	m_typeText.setFillColor(sf::Color::White);
	m_typeText.setOutlineColor(sf::Color::Black);
	m_typeText.setOutlineThickness(1.5f);
	m_typeText.setCharacterSize(30);

	// Rarity text
	m_rarityText.setPosition(barPosition + textOffset);
	m_rarityText.setOutlineColor(sf::Color::Black);
	m_rarityText.setOutlineThickness(1.f);
	m_rarityText.setCharacterSize(20);
}

void BossHealthBar::update(const MobInfo& mob, float hp) {
	m_mob = mob;
	m_hpRatio = hp;

	// Hp bar
	sf::Color fullColor = { 105, 210, 57 };
	sf::Color zeroColor = { 50, 100, 30 };
	float diameter = 2.f * m_hpBarFill.getRadius();
	float rawWidth = hp * barFillSize.x;
	float width = std::max(diameter, rawWidth);
	m_hpBarFill.setSize({ width, barFillSize.y });

	if (rawWidth < diameter) {
		float t = 1.f - (rawWidth / diameter);
		unsigned char R = (unsigned char)(fullColor.r - t * zeroColor.r);
		unsigned char G = (unsigned char)(fullColor.g - t * zeroColor.g);
		unsigned char B = (unsigned char)(fullColor.b - t * zeroColor.b);
		unsigned char A = (unsigned char)(fullColor.a - t * zeroColor.a);
		m_hpBarFill.setFillColor({ R, G, B, A });
	}
	else {
		m_hpBarFill.setFillColor(fullColor);
	}

	// Type text
	m_typeText.setString(capitalized(mob.type));
	m_typeText.setOrigin(m_typeText.getGlobalBounds().size / 2.f + m_typeText.getLocalBounds().position);

	// Rarity text
	m_rarityText.setFillColor(LIGHT_COLORS.at(mob.rarity));
	m_rarityText.setString(capitalized(mob.rarity));
	m_rarityText.setOrigin(m_rarityText.getGlobalBounds().size / 2.f + m_rarityText.getLocalBounds().position);
}

void BossHealthBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(m_hpBarBackground, states);
	target.draw(m_hpBarFill, states);
	target.draw(m_typeText, states);
	target.draw(m_rarityText, states);
}
