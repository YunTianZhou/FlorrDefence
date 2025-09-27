#include "UI.hpp"
#include "AssetManager.hpp"

UI::UI(SharedInfo& info)
	: m_info(info), m_playerStateDisplayer(info.playerState), 
	  m_shop(info), m_backpack(m_info), m_craft(m_info) {
	initComponents();
}

void UI::update() {
	// Player State Displayer
	m_playerStateDisplayer.update();

	// Menu
	m_menu.update(m_info.mouseWorldPosition);

	// Backpack / Shop / Craft / Talents
	if (m_menu.getVar() == "Backpack") m_backpack.update();
	else if (m_menu.getVar() == "Shop") m_shop.update();
	else if (m_menu.getVar() == "Craft") m_craft.update();
	else if (m_menu.getVar() == "Talents");
	else throw;
}

void UI::onEvent(const sf::Event& event) {
	if (const auto* pressedEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
		m_menu.onMouseButtonPressed(*pressedEvent);
	}
	else if (const auto* releasedEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
		std::string oldVar = m_menu.getVar();
		if (m_menu.onMouseButtonReleased(*releasedEvent)) {
			std::string newVar = m_menu.getVar();

			if (oldVar == "Backpack") m_backpack.onExit();
			else if (oldVar == "Shop") m_shop.onExit();
			else if (oldVar == "Craft") m_craft.onExit();
			else if (oldVar == "Talents");
			else throw;

			if (newVar == "Backpack") m_backpack.onEnter();
			else if (newVar == "Shop") m_shop.onEnter();
			else if (newVar == "Craft") m_craft.onEnter();
			else if (newVar == "Talents");
			else throw;
		}
	}

	if (m_menu.getVar() == "Backpack") m_backpack.onEvent(event);
	else if (m_menu.getVar() == "Shop") m_shop.onEvent(event);
	else if (m_menu.getVar() == "Craft") m_craft.onEvent(event);
	else if (m_menu.getVar() == "Talents");
	else throw;
}

void UI::updateComponents() {
	if (m_menu.getVar() == "Backpack")
		m_backpack.updateComponents();
	else if (m_menu.getVar() == "Shop")
		m_shop.updateComponents();
	else if (m_menu.getVar() == "Craft")
		m_craft.updateComponents();
	else if (m_menu.getVar() == "Talents");
	else throw;
}

void UI::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// Background
	target.draw(m_background, states);
	// Player States
	target.draw(m_playerStateDisplayer, states);
	// Menu
	target.draw(m_menu, states);

	// Backpack / Shop / Craft / Talents
	if (m_menu.getVar() == "Backpack") target.draw(m_backpack, states);
	else if (m_menu.getVar() == "Shop") target.draw(m_shop, states);
	else if (m_menu.getVar() == "Craft") target.draw(m_craft, states);
	else if (m_menu.getVar() == "Talents");
	else throw;
}

void UI::initComponents() {
	// Background
	m_background.setPosition({ 1000.f, 0.f });
	m_background.setSize({ 1700.f - 1000.f, 1100.f });
	m_background.setFillColor(LIGHT_COLORS.at("wood"));
	m_background.setOutlineColor(DARK_COLORS.at("wood"));
	m_background.setOutlineThickness(-10.f);

	// Menu
	float menuRBWidth = 130.f;
	RadioButton menuRB;
	menuRB.setPosition({ 1060.f, 170.f });
	menuRB.setSize({ menuRBWidth, 60.f });
	menuRB.setCharactorSize(24);
	menuRB.setFillColor(LIGHT_COLORS.at("wood"));
	menuRB.setOutline(DARK_COLORS.at("wood"), -5.f);

	m_menu.setVar("Backpack");
	for (std::string name : { "Backpack", "Shop", "Craft", "Talents" }) {
		menuRB.setString(name);
		menuRB.setIdentifier(name);
		m_menu.addButton(menuRB);
		menuRB.move({ menuRBWidth + 20.f, 0.f });
	}
	m_backpack.onEnter();
}
