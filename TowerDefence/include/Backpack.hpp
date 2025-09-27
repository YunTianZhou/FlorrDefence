#pragma once
#include <map>
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "ScrollBar.hpp"
#include "Card.hpp"

class BreakLine : public sf::Drawable, public sf::Transformable {
public:
	BreakLine();
	void setSize(sf::Vector2f size);
	void setRarity(const std::string& rarity, float spacing = 10.f);

private:
	void update() const;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Vector2f m_size;
	float m_spacing = 0.f;

	mutable bool m_updated = false;
	mutable float m_offset = 0.f;

	mutable sf::RoundRect m_line;
	mutable sf::Text m_text;
};

class Backpack : public sf::Drawable {
public:
	Backpack(SharedInfo& info);

	void update();
	void onEnter();
	void onExit();
	void onEvent(const sf::Event& event);
	void updateComponents() const;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void initComponents();

private:
	inline static const float startX = 1060.f;
	inline static const float startY = 260.f;
	inline static const float endY = 1090.f;
	inline static const float width = 580.f;
	inline static const float lineBreakHeight = 30.f;
	inline static const float cardLength = 100.f;
	inline static const float cardSpacing = 20.f;
	inline static const sf::FloatRect subWindowRect = sf::FloatRect({ startX, startY - 10.f }, { width + 20.f, endY - startY - 10.f});

private:
	SharedInfo& m_info;
	mutable ScrollBar m_scrollBar;
	mutable float m_contentHeight = 0.f;
	mutable std::vector<CardStack> m_cards;
	mutable std::vector<BreakLine> m_breakLines;
	mutable bool m_updated = false;
};
