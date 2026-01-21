#include <functional>
#include "Talent.hpp"
#include "AssetManager.hpp"
#include "Tools.hpp"

sf::Color blendColor(const sf::Color& a, const sf::Color& b, float wa = 0.3f) {
	wa = std::clamp(wa, 0.f, 1.f);
	float wb = 1.f - wa;

	return sf::Color(
		(unsigned char)(a.r * wa + b.r * wb),
		(unsigned char)(a.g * wa + b.g * wb),
		(unsigned char)(a.b * wa + b.b * wb),
		(unsigned char)(a.a * wa + b.a * wb)
	);
}

bool isInCircle(sf::Vector2f position, sf::Vector2f circlePosition, float radius) {
	sf::Vector2f diff = position - circlePosition;
	return diff.x * diff.x + diff.y * diff.y <= radius * radius;
}

// TalentNode
TalentNode::TalentNode(SharedInfo& info, int id)
	: m_info(info), m_id(id), m_priceText(AssetManager::getFont()) {

	m_background.setRadius(radius);
	m_background.setOrigin({ radius, radius });
	m_background.setPosition(getAttribs().position);
	m_background.setOutlineThickness(-7.f);

	m_icon.setSize({ radius * 2, radius * 2 });
	m_icon.setOrigin({ radius, radius });
	m_icon.setPosition(getAttribs().position);
	m_icon.setTexture(&AssetManager::getTalentTexture(getAttribs().type));

	m_priceBackground.setRadius(priceRadius);
	m_priceBackground.setOrigin({ priceRadius * 2, 0 });
	m_priceBackground.setPosition(getAttribs().position + sf::Vector2f(radius, -radius - 2.f));
	m_priceBackground.setFillColor(DARK_COLORS.at("disabled"));

	m_priceText.setCharacterSize(22);
	m_priceText.setFillColor(sf::Color::White);
	m_priceText.setOutlineColor(sf::Color::Black);
	m_priceText.setOutlineThickness(1.f);
	m_priceText.setPosition(m_priceBackground.getPosition() + sf::Vector2f(-priceRadius, priceRadius));
}

void TalentNode::activate() {
	m_isActive = true;
	m_colorUpdated = false;
}

void TalentNode::setPrice(int price) {
	m_price = price;

	std::string price_str = std::to_string(price);
	m_priceText.setString(price_str);
	m_priceText.setRotation(sf::degrees(0.f));
	m_priceText.setOrigin(m_priceText.getGlobalBounds().size / 2.f + m_priceText.getLocalBounds().position);
	m_priceText.setRotation(sf::degrees(12.f));

	m_colorUpdated = false;
}

void TalentNode::onMouseButtonPressed(sf::Event::MouseButtonPressed event, sf::Vector2f offset) {
	sf::Vector2f position = getAttribs().position + offset;

	m_held = isInCircle(m_info.mouseWorldPosition, position, radius);
}

bool TalentNode::onMouseButtonReleased(sf::Event::MouseButtonReleased event, sf::Vector2f offset) {
	sf::Vector2f position = getAttribs().position + offset;

	bool pressed = m_held && isInCircle(m_info.mouseWorldPosition, position, radius);
	m_held = false;

	return pressed;
}

void TalentNode::updateColor() const {
	if (m_isActive) {
		m_background.setFillColor(LIGHT_COLORS.at(getAttribs().rarity));
		m_background.setOutlineColor(DARK_COLORS.at(getAttribs().rarity));
	}
	else {
		if (m_price <= m_info.playerState.talent) {
			m_priceText.setFillColor(sf::Color::White);

			const sf::Color& c1 = LIGHT_COLORS.at(getAttribs().rarity);
			const sf::Color& c2 = LIGHT_COLORS.at("disabled");
			sf::Color fillColor = blendColor(c1, c2);
			m_background.setFillColor(fillColor);

			const sf::Color& d1 = DARK_COLORS.at(getAttribs().rarity);
			const sf::Color& d2 = DARK_COLORS.at("disabled");
			sf::Color outlineColor = blendColor(d1, d2);
			m_background.setOutlineColor(outlineColor);
			m_priceBackground.setFillColor(outlineColor);
		}
		else {
			m_priceText.setFillColor(sf::Color(255, 102, 102));

			m_background.setFillColor(LIGHT_COLORS.at("disabled"));
			m_background.setOutlineColor(DARK_COLORS.at("disabled"));
			m_priceBackground.setFillColor(DARK_COLORS.at("disabled"));
		}
	}

	m_colorUpdated = true;
}

void TalentNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!m_colorUpdated)
		updateColor();
	
	target.draw(m_background, states);
	target.draw(m_icon, states);

	if (!m_isActive) {
		target.draw(m_priceBackground, states);
		target.draw(m_priceText, states);
	}
}

// TalentEdge
TalentEdge::TalentEdge(SharedInfo& info, int prev, int curr) 
	: m_info(info), m_prev(prev), m_curr(curr) {

	const TalentAttribs& prev_attribs = TALENT_ATTRIBS[prev];
	const TalentAttribs& curr_attribs = TALENT_ATTRIBS[curr];

	sf::Vector2f diff = curr_attribs.position - prev_attribs.position;
	float length = std::sqrt(diff.x * diff.x + diff.y * diff.y);
	float angle = std::atan2(diff.y, diff.x);

	m_line.setSize(sf::Vector2f(length, thickness));
	m_line.setPosition(prev_attribs.position);
	m_line.setRotation(sf::radians(angle));
	m_line.setFillColor(DARK_COLORS.at("wood"));
}

void TalentEdge::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(m_line, states);
}

// Talent
Talent::Talent(SharedInfo& info)
	: m_info(info) {
	initComponents();
}

void Talent::update() {
	if (prevTalent != m_info.playerState.talent)
		m_updated = false;

	prevTalent = m_info.playerState.talent;

	if (!m_updated) 
		updateComponents();

	// Scroll Bar
	m_scrollBar.update(m_info.mouseWorldPosition);
}

void Talent::onEnter() {
	m_updated = false;
}

void Talent::onExit() {
	m_updated = false;
}

void Talent::onEvent(const sf::Event& event) {
	if (const auto* pressedEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
		m_scrollBar.onMouseButtonPressed(*pressedEvent);

		for (auto& node : m_nodes)
			node.onMouseButtonPressed(*pressedEvent, getOffset());
	}
	else if (const auto* releasedEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
		m_scrollBar.onMouseButtonReleased(*releasedEvent);

		for (auto& node : m_nodes)
			if (node.onMouseButtonReleased(*releasedEvent, getOffset()))
				buyTalent(TALENT_ID_TO_INDEX[node.getAttribs().id]);

	}
	else if (const auto* scrolledEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
		if (!m_info.input.mouseLeftButton && subWindowRect.contains(m_info.mouseWorldPosition)) {
			m_scrollBar.onMouseWheelScrolled(*scrolledEvent);
		}
	}
}

void Talent::updateComponents() {
	std::function<void(int, int)> dfs = [&](int curr, int cost) {
		if (!m_nodes[curr].isActive()) {
			cost += m_nodes[curr].getAttribs().xp_cost;
			m_nodes[curr].setPrice(cost);
		}

		for (int next : m_graph[curr])
			dfs(next, cost);
	};

	for (int start : m_starts)
		dfs(start, 0);

	m_updated = true;
}

sf::Vector2f Talent::getOffset() const {
	return { subWindowRect.position.x,
			 subWindowRect.position.y - m_scrollBar.getOffset() };
}

void Talent::buyTalent(int id) {
	TalentNode& node = m_nodes[id];

	if (node.isActive() || node.getPrice() > m_info.playerState.talent)
		return;

	m_info.playerState.talent -= node.getPrice();
	int i = id;
	while (!m_nodes[i].isActive()) {
		m_nodes[i].activate();
		Buff& buff = m_info.playerState.talentBuff.get(m_nodes[i].getAttribs().buff_type);
		buff.add(m_nodes[i].getAttribs().buff_value);

		const auto& prev = m_nodes[i].getAttribs().prev_id;
		if (!prev) break;
		i = TALENT_ID_TO_INDEX[*prev];
	}

	m_updated = false;
}

void Talent::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	sf::View view = target.getView();
	view.setScissor(getScissorRect(target, subWindowRect));
	target.setView(view);

	sf::Transform transform;
	transform.translate(getOffset());
	sf::Transform original_transform = states.transform;
	states.transform *= transform;

	// Talent Edges
	for (const auto& edge : m_edges)
		target.draw(edge, states);

	// Talent Nodes
	for (const auto& node : m_nodes)
		target.draw(node, states);

	states.transform = original_transform;

	view.setScissor({ { 0.f, 0.f }, { 1.f, 1.f } });
	target.setView(view);

	// Scroll bar
	if (contentHeight > m_scrollBar.getViewHeight())
		target.draw(m_scrollBar, states);
}

void Talent::initComponents() {
	// Talent nodes
	m_graph.resize(TALENT_ATTRIBS.size());
	for (int id = 0; id < TALENT_ATTRIBS.size(); id++) {
		const TalentAttribs& attibs = TALENT_ATTRIBS[id];
		m_nodes.emplace_back(m_info, id);
		if (attibs.prev_id.has_value()) {
			int prev_id = TALENT_ID_TO_INDEX[*attibs.prev_id];
			m_graph[prev_id].push_back(id);
			m_edges.emplace_back(m_info, prev_id, id);
		}
		else {
			m_starts.push_back(id);
		}
	}

	// Scroll bar
	m_scrollBar.setPosition({ 1640.f, subWindowRect.position.y });
	m_scrollBar.setSize({ 10.f, subWindowRect.size.y });
	m_scrollBar.setFillColor(DARK_COLORS.at("wood"));
	m_scrollBar.setViewHeight(subWindowRect.size.y - 10.f);
	m_scrollBar.setContentHeight(contentHeight);
}
