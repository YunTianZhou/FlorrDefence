#include <format>
#include "Craft.hpp"
#include "Tools.hpp"
#include "AssetManager.hpp"

void CraftInfo::reset(const std::string& rarity) {
	TimeRange craftTimeRange = CRAFT_TIME_RANGES.at(rarity);
	successCount = successCount;
	remaningCount = remaningCount;
	processTime = sf::seconds(randomUniform(craftTimeRange.lower.asSeconds(),
		craftTimeRange.upper.asSeconds()));
	elapsedTime = sf::seconds(0.f);
	successCount = 0;
	remaningCount = 0;
	clock.restart();
}

Craft::Craft(SharedInfo& info) 
	: m_info(info), m_craftProbText(AssetManager::getFont()) {
	initComponents();
	collapseCards();
}

void Craft::update() {
	// Craft
	if (m_craftState == "crafting") {
		sf::Time dt = m_craftInfo.clock.restart();
		m_craftInfo.elapsedTime += dt;

		if (m_craftInfo.elapsedTime >= m_craftInfo.processTime) {
			endCraft();
		}
		else {
			float elapsedSec = m_craftInfo.elapsedTime.asSeconds();
			float startSpeed = 360.f;
			float spinSpeed = startSpeed + 3.f * elapsedSec * elapsedSec;

			m_craftTableAngle += sf::degrees(spinSpeed * dt.asSeconds());

			float frequency = 1.0f + 0.03f * elapsedSec; // slower acceleration
			float phase = elapsedSec * frequency * 2.f * 3.14159265f + 3.14159265f / 2.f;

			float t = std::sin(phase);
			t = t * t;
			t = std::clamp(t, 0.f, 1.f);

			float minScale = 0.4f;
			float maxScale = 1.0f;
			m_craftTableRadiusMultiple = minScale + t * (maxScale - minScale);
		}
	}
	
	// Craft Button
	if (m_craftState == "preparing" && m_craftStack.count >= 5) {
		const std::string& rarity = m_craftStack.card.rarity;
		int index = (int)std::distance(RARITIES.begin(), std::find(RARITIES.begin(), RARITIES.end(), rarity));
		const std::string& nextRarity = RARITIES[index + 1];
		m_craftButton.setDisabled(false);
		m_craftButton.setFillColor(LIGHT_COLORS.at(nextRarity));
		m_craftButton.setOutline(DARK_COLORS.at(nextRarity), 5.f);
	}
	else {
		m_craftButton.setDisabled(true);
	}

	// Craft Button
	m_craftButton.update(m_info.mouseWorldPosition);

	// Scroll Bar
	float prevOffset = m_scrollBar.getOffset();
	m_scrollBar.update(m_info.mouseWorldPosition);
	if (prevOffset != m_scrollBar.getOffset())
		m_updated = false;
}

void Craft::onEnter() {
	if (m_info.playerState.backpack.getCount(m_craftStack.card) < m_craftStack.count)
		collapseCards();

	m_scrollBar.setOffset(0.f);
	m_scrollBar.setContentHeight(0.f);
	m_updated = false;
}

void Craft::onExit() {
	if (m_craftState == "crafting")
		endCraft();
}

void Craft::onEvent(const sf::Event& event) {
	if (const auto* pressedEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
		m_craftButton.onMouseButtonPressed(*pressedEvent);
		m_scrollBar.onMouseButtonPressed(*pressedEvent);

		if (pressedEvent->button == sf::Mouse::Button::Left) {
			sf::Vector2f mousePosition = m_info.mouseWorldPosition;
			if (!subWindowRect.contains(mousePosition))
				mousePosition = { -1.f, -1.f };  // Outside of sub window
			for (CardStack& stack : m_cards) {
				if (stack.getRect().contains(mousePosition)) {
					CardStackInfo cards = stack.getInfo();
					if (!m_info.input.keyShift)
						cards.count = std::min(cards.count, 5 - 
							(m_craftStack.card == cards.card && m_craftStack.count < 5 && 
								m_craftState != "succeeded" ? m_craftStack.count : 0));
					insertCards(cards);
				}
			}

			sf::Vector2f cardSquare = { cardLength, cardLength };
			for (sf::FloatRect craftCardRect : m_craftCardRects) {
				if (craftCardRect.contains(m_info.mouseWorldPosition)) {
					collapseCards();
				}
			}
		}
	}
	else if (const auto* releasedEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (m_craftButton.onMouseButtonReleased(*releasedEvent))
			startCraft();
		m_scrollBar.onMouseButtonReleased(*releasedEvent);
	}
	else if (const auto* scrolledEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
		if (!m_info.input.mouseLeftButton && subWindowRect.contains(m_info.mouseWorldPosition)) {
			m_scrollBar.onMouseWheelScrolled(*scrolledEvent);
			m_updated = false;
		}
	}
}

void Craft::collapseCards() {
	if (m_craftState == "crafting") return;
	m_craftStack = defaultCraftStack;
	m_craftState = "preparing";

	m_craftProbText.setString("?% success chance");
	m_craftProbText.setOrigin(m_craftProbText.getGlobalBounds().size / 2.f + 
		                       m_craftProbText.getLocalBounds().position);

	m_updated = false;
}

void Craft::insertCards(const CardStackInfo& cards) {
	if (m_craftState == "crafting" || cards.card.rarity == "super" || cards.card.rarity == "unique")
		return;
	if (m_craftState != "succeeded" && m_craftStack.card == cards.card) {
		if (m_craftStack.count + cards.count < 5)
			return;
		m_craftStack.count += cards.count;
	}
	else if (cards.count >= 5) {
		m_craftStack = cards;
		m_craftProbText.setString(std::format("{}% success chance", CRAFT_PROBS.at(cards.card.rarity) * 100.f));
		m_craftProbText.setOrigin(m_craftProbText.getGlobalBounds().size / 2.f +
		m_craftProbText.getLocalBounds().position);
	}
	else return;

	m_craftState = "preparing";
	m_updated = false;
}

void Craft::startCraft() {
	if (m_craftState != "preparing") return;
	if (m_craftStack.count < 5) return;

	float prob = CRAFT_PROBS.at(m_craftStack.card.rarity);
	int successCount = 0;
	int remaningCount = m_craftStack.count;
	while (remaningCount >= 5) {
		if (randomUniform(0.f, 1.f) <= prob) {
			successCount += 1;
			remaningCount -= 5;
		}
		else {
			remaningCount -= randomInt(1, 4);
		}
	}
	m_craftInfo.reset(m_craftStack.card.rarity);
	m_craftInfo.successCount = successCount;
	m_craftInfo.remaningCount = remaningCount;

	m_craftState = "crafting";
}

void Craft::endCraft() {
	assert(m_craftState == "crafting");

	const std::string& rarity = m_craftStack.card.rarity;
	const std::string& nextRarity = RARITIES[std::distance(RARITIES.begin(), std::find(RARITIES.begin(), RARITIES.end(), rarity)) + 1];

	m_info.playerState.backpack.add({ m_craftStack.card, m_craftInfo.remaningCount - m_craftStack.count });
	m_info.playerState.backpack.add({ { nextRarity, m_craftStack.card.type }, m_craftInfo.successCount });

	if (m_craftInfo.successCount > 0) {
		m_craftState = "succeeded";
		m_craftStack = { { nextRarity, m_craftStack.card.type }, m_craftInfo.successCount };
	}
	else {
		m_craftState = "failed";
		m_craftStack.count = m_craftInfo.remaningCount;
	}

	m_craftTableRadiusMultiple = 1.f;
	m_updated = false;
}

void Craft::updateComponents() const {
	m_cards.clear();
	m_emptyPositions.clear();
	m_card.setOrigin({ 0.f, 0.f });
	m_empty.setOrigin({ 0.f, 0.f });

	const BackpackInfo& backpack = m_info.playerState.backpack;

	float y = startY - m_scrollBar.getOffset();
	for (const std::string& type : TOWER_TYPES) {
		if (backpack.getTypeCount(type) == 0) continue;
		if (y + cardLength >= startY && y <= endY) {
			float x = startX + width - cardLength;
			for (auto it = RARITIES.rbegin(); it != RARITIES.rend(); it++) {
				const std::string& rarity = *it;
				if (rarity == "unique") continue;
				CardInfo card = { rarity, type };
				int realCount = backpack.getCount(card);
				int count = realCount;
				if (m_craftState != "succeeded" && card == m_craftStack.card) {
					assert(count >= m_craftStack.count);
					count -= m_craftStack.count;
				}
				if (count > 0) {
					m_card.setPosition({ x, y });
					m_card.setCard(card);
					m_card.setCount(count);
					m_card.setDisabled(realCount < 5 && rarity != "super");
					m_cards.push_back(m_card);
				}
				else {
					m_emptyPositions.push_back({ x, y });
				}
				x -= cardLength + cardSpacing;
			}
		}
		y += cardLength + cardSpacing;
	}
	y -= cardSpacing;
	m_contentHeight = y - (startY - m_scrollBar.getOffset());
	m_scrollBar.setContentHeight(m_contentHeight);

	m_updated = true;
}

void Craft::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// Crafting table
	m_craftCardRects.clear();
	sf::Vector2f center = { 1250.f, 380.f };
	sf::Vector2f cardSquare(cardLength, cardLength);

	m_card.setOrigin(cardSquare / 2.f);
	m_empty.setOrigin(cardSquare / 2.f);
	if (m_craftStack != defaultCraftStack) m_card.setCard(m_craftStack.card);
	m_card.setDisabled(false);
	if (m_craftState == "succeeded") {
		assert(m_craftStack.count > 0);
		float scale = 1.5f;
		m_craftCardRects.emplace_back(center - (cardSquare * scale) / 2.f, cardSquare * scale);
		m_card.setScale({ scale, scale });
		m_card.setPosition(center);
		m_card.setCount(m_craftStack.count);
		target.draw(m_card, states);
		m_card.setScale({ 1.f, 1.f });
	}
	else {
		constexpr float PI = 3.14159265f;
		float radius = 92.f * m_craftTableRadiusMultiple;

		int base = m_craftStack.count / 5;
		int rem = m_craftStack.count % 5;
		for (int i = 0; i < 5; i++) {
			int count = base + (i < rem ? 1 : 0);
			sf::Angle angle = sf::radians(2 * PI * i / 5) - sf::degrees(90.f) + m_craftTableAngle;
			sf::Vector2f point = center + sf::Vector2f(std::cos(angle.asRadians()), std::sin(angle.asRadians())) * radius;

			if (count > 0) {
				m_craftCardRects.emplace_back(point - cardSquare / 2.f, cardSquare);
				m_card.setPosition(point);
				m_card.setCount(count);
				target.draw(m_card, states);
			}
			else {
				m_empty.setPosition(point);
				target.draw(m_empty, states);
			}
		}
	}

	// Craft Button
	target.draw(m_craftButton, states);

	// Craft Probability Text
	target.draw(m_craftProbText, states);

	// Cards
	sf::View view = target.getView();
	view.setScissor(getScissorRect(target, subWindowRect));
	target.setView(view);

	if (!m_updated)
		updateComponents();

	m_empty.setOrigin({ 0.f, 0.f });
	for (sf::Vector2f emptyPosition : m_emptyPositions) {
		m_empty.setPosition(emptyPosition);
		target.draw(m_empty, states);
	}

	for (const CardStack& stack : m_cards)
		target.draw(stack, states);

	view.setScissor({ { 0.f, 0.f }, { 1.f, 1.f } });
	target.setView(view);

	// Scroll bar
	if (m_contentHeight > m_scrollBar.getViewHeight())
		target.draw(m_scrollBar, states);
}

void Craft::initComponents() {
	// Card
	m_card.setLength(cardLength);

	// Empty
	m_empty.setSize({ cardLength, cardLength });
	m_empty.setRadius(cardLength * (30.f / 922.f));
	m_empty.setFillColor(DARK_COLORS.at("wood"));

	// Craft
	m_craftButton.setPosition({ startX + 400.f, 360.f });
	m_craftButton.setSize({ 70.f, 32.f });
	m_craftButton.setString("Craft");
	m_craftButton.setCharactorSize(18);
	m_craftButton.setOutline(sf::Color::Transparent, 5.f);

	m_craftProbText.setPosition({ startX + 400.f + 70.f / 2.f, 405.f });
	m_craftProbText.setCharacterSize(14);
	m_craftProbText.setFillColor(sf::Color::White);
	m_craftProbText.setOutlineColor(sf::Color::Black);
	m_craftProbText.setOutlineThickness(0.8f);

	// Scroll bar
	m_scrollBar.setPosition({ 1640.f + 20.f, subWindowRect.position.y });
	m_scrollBar.setSize({ 10.f, subWindowRect.size.y });
	m_scrollBar.setFillColor(DARK_COLORS.at("wood"));
	m_scrollBar.setViewHeight(subWindowRect.size.y - 10.f);
}
