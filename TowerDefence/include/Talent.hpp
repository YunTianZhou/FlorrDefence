#pragma once
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "RoundRect.hpp"
#include "ScrollBar.hpp"
#include "Button.hpp"
#include "Constants.hpp"

class TalentNode : public sf::Drawable {
public:
	TalentNode(SharedInfo& info, int id);

	void activate();
	void setPrice(int price);

	bool isActive() const { return m_isActive; }
	int getPrice() const { return m_price; }

	void onMouseButtonPressed(sf::Event::MouseButtonPressed event, sf::Vector2f offset);
	bool onMouseButtonReleased(sf::Event::MouseButtonReleased event, sf::Vector2f offset);

	const TalentAttribs& getAttribs() const { return TALENT_ATTRIBS.at(m_id); };

private:
	void updateColor() const;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;;

private:
	inline static const float radius = 42.f;
	inline static const float priceRadius = 15.f;

private:
	int m_id = 0;
	SharedInfo& m_info;

	mutable sf::CircleShape m_background;
	sf::RectangleShape m_icon;
	mutable sf::CircleShape m_priceBackground;
	mutable sf::Text m_priceText;

	bool m_isActive = false;
	int m_price = 0;
	bool m_held = false;

	mutable bool m_colorUpdated = false;
};

class TalentEdge : public sf::Drawable {
public:
	TalentEdge(SharedInfo& info, int prev, int curr);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	inline static const float thickness = 10.f;

private:
	SharedInfo& m_info;

	int m_prev;
	int m_curr;

	sf::RectangleShape m_line;
};

class Talent : public sf::Drawable {
public:
	Talent(SharedInfo& info);

	void update();
	void onEnter();
	void onExit();
	void onEvent(const sf::Event& event);
	void updateComponents();

private:
	sf::Vector2f getOffset() const;
	void buyTalent(int id);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void initComponents();

private:
	inline static const float startX = 1060.f;
	inline static const float startY = 260.f;
	inline static const float endY = 1090.f;
	inline static const float width = 580.f;
	inline static const float contentHeight = 900.f;
	inline static const sf::FloatRect subWindowRect = sf::FloatRect({ startX, startY - 10.f }, { width, endY - startY - 10.f });

private:
	SharedInfo& m_info;
	ScrollBar m_scrollBar;

	std::vector<TalentNode> m_nodes;
	std::vector<TalentEdge> m_edges;
	std::vector<int> m_starts;
	std::vector<std::vector<int>> m_graph;

	int prevTalent = 0;
	bool m_updated = false;
};
