#pragma once
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include "RoundRect.hpp"
#include "Constants.hpp"

class LabelEntry : public sf::Drawable, public sf::Transformable {
public:
	LabelEntry(
		const std::string& labelText,
		const std::string& valueText,
		unsigned int charSize,
		sf::Color labelColor = sf::Color::Yellow,
		sf::Color valueColor = sf::Color::White
	);

	void setPosition(sf::Vector2f pos);
	void setValue(const std::string& valueText);

private:
	sf::Text label;
	sf::Text value;

	void updateLayout();

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class CardDescription : public sf::Drawable {
public:
	CardDescription();

	void set(const CardInfo& card, sf::Vector2f cardCenter, float cardSize);

	void reset() { m_isVerified = false; }
	bool isVerified() const { return m_isVerified; }

private:
	void load_data();
	std::string parseAttrib(const std::string& value, const std::string& type);

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
	static inline const float labelInterval = 18.f;

	static inline const unsigned int titleCharSize = 28;
	static inline const unsigned int contentCharSize = 16;
	static inline const float contentLineSpacing = 1.15f;

private:
	CardInfo m_card;
	sf::Vector2f m_cardCenter;
	float m_cardSize = 0.f;
	bool m_isVerified = false;
	const float m_lineSpacing;

	sf::Vector2f m_size;
	sf::Vector2f m_position;

	sf::RoundRect m_background;
	sf::Text m_title;
	sf::Text m_rarity;
	sf::Text m_content;
	std::vector<LabelEntry> m_labels;

	nlohmann::json m_info;
	std::unordered_map<std::string, sf::Color> m_colorTable;
};
