#pragma once
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "Button.hpp"
#include "ScrollBar.hpp"
#include "PlayerStateDisplayer.hpp"
#include "Backpack.hpp"
#include "Shop.hpp"
#include "Craft.hpp"

class UI : public sf::Drawable {
public:
	UI(SharedInfo& info);

	void update();
	void onEvent(const sf::Event& event);
	void updateComponents();

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void initComponents();

private:
	SharedInfo& m_info;

	sf::RectangleShape m_background;
	PlayerStateDisplayer m_playerStateDisplayer;
	RadioButtonGroup m_menu;
	Backpack m_backpack;
	Shop m_shop;
	Craft m_craft;
};
