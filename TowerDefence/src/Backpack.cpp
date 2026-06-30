#include "Backpack.hpp"
#include "AssetManager.hpp"
#include "Constants.hpp"
#include "Tools.hpp"

BreakLine::BreakLine() 
	: m_text(AssetManager::getFont()) {
	m_line.setFillColor(DARK_COLORS.at("wood"));
	m_text.setCharacterSize(20);
	m_text.setOutlineColor(sf::Color::Black);
	m_text.setOutlineThickness(1.f);
}

void BreakLine::setSize(sf::Vector2f size) {
	m_size = size;
	m_line.setSize(size);
	m_line.setRadius(size.y * 0.5f);
	m_updated = false;
}

void BreakLine::setRarity(const std::string& rarity, float spacing) {
	m_text.setString(capitalized(rarity));
	m_text.setFillColor(LIGHT_COLORS.at(rarity));
	m_spacing = spacing;
	m_updated = false;
}

void BreakLine::update() const {
	assert(m_size != sf::Vector2f());
	m_text.setOrigin(m_text.getGlobalBounds().size / 2.f + m_text.getLocalBounds().position);
	m_text.setPosition(m_size / 2.f);

	float textWidth = m_text.getGlobalBounds().size.x;
	m_line.setSize({ (m_size.x - textWidth) / 2.f - m_spacing, m_size.y });
	m_offset = (m_size.x + textWidth) / 2.f + m_spacing;

	m_updated = true;
}

void BreakLine::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	if (!m_updated)
		update();
	target.draw(m_line, states);
	target.draw(m_text, states);
	states.transform.translate({ m_offset, 0.f });
	target.draw(m_line, states);
}

Backpack::Backpack(SharedInfo& info)
	: m_info(info) {
	initComponents();
}

void Backpack::update() {
	// Card description
	if (!m_info.draggedCard.has_value() &&
		subWindowRect.contains(m_info.mouseWorldPosition)) {
		for (const Card& card : m_cards) {
			if (card.getRect().contains(m_info.mouseWorldPosition)) {
				sf::Vector2f center = card.getPosition() + sf::Vector2f(cardLength, cardLength) / 2.f;
				m_info.cardDescription.set(card.getCard(), center, cardLength);
			}
		}
	}

	// Scroll bar
	float prevOffset = m_scrollBar.getOffset();
	m_scrollBar.update(m_info.mouseWorldPosition);
	if (prevOffset!= m_scrollBar.getOffset())
		m_updated = false;
}

void Backpack::onEnter() {
	m_scrollBar.setContentHeight(0.f);
	m_scrollBar.setOffset(0.f);
	m_updated = false;
}

void Backpack::onExit() {

}

void Backpack::onEvent(const sf::Event& event) {
	if (const auto* pressedEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressedEvent->button == sf::Mouse::Button::Left && !m_info.draggedCard.has_value()) {
			for (const CardStack& stack : m_cards) {
				assert(stack.getCount() > 0);
				if (stack.getRect().contains(m_info.mouseWorldPosition)) {
					m_info.playerState.backpack.add({ stack.getCard(), -1 });
					m_info.draggedCard = DraggedCard(stack.getCard());
					m_updated = false;
					break;
				}
			}
		}
		else if (pressedEvent->button == sf::Mouse::Button::Right && !m_info.draggedCard.has_value()) {
			for (const CardStack& stack : m_cards) {
				assert(stack.getCount() > 0);
				if (stack.getRect().contains(m_info.mouseWorldPosition)) {
					if (m_info.input.keyShift)
						m_info.placeRequest = stack.getInfo();
					else
						m_info.placeRequest = { stack.getCard(), 1 };
					m_updated = false;
					break;
				}
			}
		}
		m_scrollBar.onMouseButtonPressed(*pressedEvent);
	}
	else if (const auto* releasedEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
		m_scrollBar.onMouseButtonReleased(*releasedEvent);
	}
	else if (const auto* scrolledEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
		if (!m_info.input.mouseLeftButton && subWindowRect.contains(m_info.mouseWorldPosition)) {
			m_scrollBar.onMouseWheelScrolled(*scrolledEvent);
			m_updated = false;
		}
	}
}

void Backpack::updateComponents() const {
	m_breakLines.clear();
	m_cards.clear();

	float y = startY - m_scrollBar.getOffset();
	CardStack stack;
	stack.setLength(cardLength);
	auto& backpack = m_info.playerState.backpack;

	for (auto it = RARITIES.rbegin(); it != RARITIES.rend(); it++) {
		const std::string& rarity = *it;
		if (backpack.getRarityCount(rarity) == 0)
			continue;

		if (y + lineBreakHeight >= startY && y <= endY) {
			BreakLine line;
			line.setSize({ 580.f, 8.f });
			line.setRarity(rarity);
			line.setPosition({ startX, y });
			m_breakLines.push_back(line);
		}
		y += lineBreakHeight;

		std::vector<CardStackInfo> info;
		for (const std::string& type : TOWER_TYPES) {
			int count = backpack.getCount({ rarity, type });
			if (count > 0)
				info.emplace_back(CardInfo(rarity, type), count);
		}

		for (int i = 0; i < info.size(); i += 5) {
			if (y + cardLength >= startY && y <= endY) {
				int count = std::min(5, (int)(info.size()) - i);
				float x = startX + width - cardLength - (width - count * cardLength - (count - 1) * cardSpacing) / 2.f;
				for (int j = i + count - 1; j >= i; j--) {
					stack.setPosition({ x, y });
					stack.setInfo(info[j]);
					m_cards.push_back(stack);
					x -= cardLength + cardSpacing;
				}
			}
			y += cardLength + cardSpacing;
		}
	}
	y -= cardSpacing;
	m_contentHeight = y - (startY - m_scrollBar.getOffset());
	m_scrollBar.setContentHeight(m_contentHeight);

	m_updated = true;
}

void Backpack::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// Scroll bar
	if (m_scrollBar.getViewHeight() < m_scrollBar.getContentHeight()) {
		target.draw(m_scrollBar, states);
	}

	// Cards
	sf::View view = target.getView();
	view.setScissor(getScissorRect(target, subWindowRect));
	target.setView(view);

	if (!m_updated)
		updateComponents();

	for (const auto& line : m_breakLines)
		target.draw(line, states);

	for (const auto& card : m_cards)
		target.draw(card, states);

	view.setScissor({ { 0.f, 0.f }, { 1.f, 1.f } });
	target.setView(view);
}

void Backpack::initComponents() {
	// Scroll bar
	m_scrollBar.setPosition({ 1640.f + 20.f, subWindowRect.position.y });
	m_scrollBar.setSize({ 10.f, subWindowRect.size.y });
	m_scrollBar.setFillColor(DARK_COLORS.at("wood"));
	m_scrollBar.setViewHeight(subWindowRect.size.y - 10.f);
}
