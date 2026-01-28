#pragma once
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "ScrollBar.hpp"
#include "Button.hpp"
#include "RoundRect.hpp"
#include "Card.hpp"

struct CraftInfo {
	int successCount = 0;
	int remaningCount = 0;
	sf::Time processTime;
	sf::Time elapsedTime = sf::seconds(0);

	void reset(const std::string& rarity);
};

class Craft : public sf::Drawable {
public:
	Craft(SharedInfo& info);

	void update();
	void onEnter();
	void onExit();
	void onEvent(const sf::Event& event);
	void updateComponents() const;

private:
	void collapseCards();
	void insertCards(const CardStackInfo& cards);
	void startCraft();
	void endCraft();
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void initComponents();

private:
	inline static const float startX = 1060.f;
	inline static const float startY = 530.f;
	inline static const float endY = 1090.f;
	inline static const float width = 580.f;
	inline static const float cardLength = 62.f;
	inline static const float cardSpacing = 12.f;
	inline static const sf::FloatRect subWindowRect = sf::FloatRect({ startX, startY - 10.f }, { width + 20.f, endY - startY - 10.f });
	inline static CardStackInfo defaultCraftStack = { { "", "" }, 0 };
	inline static int craftLimit = 500000;

private:
	SharedInfo& m_info;
	mutable ScrollBar m_scrollBar;
	mutable float m_contentHeight = 0.f;

	CardStackInfo m_craftStack = defaultCraftStack;
	std::string m_craftState = "preparing";  // preparing, crafting, succeeded, failed
	CraftInfo m_craftInfo;
	sf::Angle m_craftTableAngle = sf::degrees(0.f);
	float m_craftTableRadiusMultiple = 1.f;
	ClickButton m_craftButton;
	sf::Text m_craftProbText;

	mutable CardStack m_card;
	mutable sf::RoundRect m_empty;

	mutable std::vector<CardStack> m_cards;
	mutable std::vector<sf::FloatRect> m_craftCardRects;
	mutable std::vector<sf::Vector2f> m_emptyPositions;
	mutable bool m_updated = false;
};
