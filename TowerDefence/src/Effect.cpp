#include "Effect.hpp"
#include "Tools.hpp"

sf::Vector2f addJitter(sf::Vector2f pos, float delta) {
	float dx = randomUniform(-delta, delta);
	float dy = randomUniform(-delta, delta);

	return pos + sf::Vector2f(dx, dy);
}

// Effect
Effect::Effect(const SharedInfo& info) 
	: m_info(info) {}

// Lightning Effect
LightningEffect::LightningEffect(const SharedInfo& info, sf::Vector2f root, int rootConnected, std::vector<sf::Vector2f> positions)
	: Effect(info) {
	const float delta = 8.f;
	root = addJitter(root, delta);
	for (auto& pos : positions)
		pos = addJitter(pos, delta);

	buildLine(root, positions[rootConnected]);
	for (int i = 1; i < positions.size(); i++)
		buildLine(positions[i - 1], positions[i]);
}

void LightningEffect::update() {
	m_timer += m_info.dt;
	float t = m_timer.asSeconds();

	float ratio = std::min(t / duration, 1.f);
	unsigned char alpha = (unsigned char)(255 * (1.f - ratio));

	for (auto& line : m_lines) {
		sf::Color c = line.getFillColor();
		c.a = alpha;
		line.setFillColor(c);
	}
}

bool LightningEffect::isDone() {
	return m_timer.asSeconds() >= duration;
}

void LightningEffect::buildLine(sf::Vector2f start, sf::Vector2f end) {
	sf::Vector2f diff = end - start;
	float length = std::sqrt(diff.x * diff.x + diff.y * diff.y);

	sf::RectangleShape line;
	line.setSize(sf::Vector2f(length, thinkness));
	line.setFillColor(sf::Color::White);
	line.setOrigin({ 0.f, thinkness / 2.f });
	line.setPosition(start);
	line.setRotation(sf::radians(std::atan2(diff.y, diff.x)));

	m_lines.push_back(std::move(line));
}

void LightningEffect::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	for (auto& line : m_lines)
		target.draw(line, states);
}
