#pragma once
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include "RoundRect.hpp"
#include "RichText.hpp"
#include "Constants.hpp"

class TalentDescription : public sf::Drawable {
public:
	TalentDescription();

	void set(const CardInfo& talent, sf::Vector2f talentCenter, float talentRadius);

	void reset() { m_isVerified = false; }
	bool isVerified() const { return m_isVerified; }

private:
	void loadData();

	void updateText();
	void updateTextPosition();
	void updateBackgroundPosition();

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	static inline const float outsidePadding = 8.f;
	static inline const float leftPadding = 10.f;
	static inline const float rightPadding = 10.f;
	static inline const float topPadding = 4.f;
	static inline const float bottomPadding = 8.f;

	static inline const float minWidth = 324.f;
	static inline const float rarityInterval = 9.f;
	static inline const float contentInterval = 22.f;

	static inline const unsigned int titleCharSize = 28;
	static inline const unsigned int contentCharSize = 16;
	static inline const float contentLineSpacing = 1.15f;

private:
	CardInfo m_talent;
	sf::Vector2f m_talentCenter;
	float m_talentRadius = 0.f;
	const float m_lineSpacing;
	bool m_isVerified = false;

	sf::Vector2f m_size;
	sf::Vector2f m_position;

	sf::RoundRect m_background;
	sf::Text m_title;
	sf::Text m_rarity;
	sf::RichText m_content;

	nlohmann::json m_data;
	std::unordered_map<std::string, sf::Color> m_colorTable;
};
