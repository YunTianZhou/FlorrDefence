#pragma once
#include <cmath>
#include "Card.hpp"

class DraggedCard : public sf::Drawable {
public:
	explicit DraggedCard(const CardInfo& card, std::optional<sf::Vector2i> startSquare = std::nullopt)
		: m_startSquare(startSquare) {
		m_card.setCard(card);
		m_card.setLength(100.f);
		m_card.setOrigin({ 50.f, 50.f });
	}

	bool update(sf::Vector2f mouseWorldPosition, sf::Time dt) {
		if (isRetreating()) {
			sf::Vector2f current = m_card.getPosition();
			sf::Vector2f toTarget = retreatTarget - current;
			float dist = std::hypot(toTarget.x, toTarget.y);
			float step = retreatSpeed * dt.asSeconds();

			if (dist <= step) {
				m_card.setPosition(retreatTarget);
				m_isRetreating = false;
				return true;
			}
			else {
				sf::Vector2f direction = toTarget / dist;
				m_card.move(direction * step);
				return false;
			}
		}
		else {
			m_card.setPosition(mouseWorldPosition);
			return false;
		}
	}

	void setPosition(sf::Vector2f position) { m_card.setPosition(position); }
	void startRetreat() { m_isRetreating = true; }
	bool isRetreating() const { return m_isRetreating; }
	CardInfo getCard() const { return m_card.getCard(); }
	std::optional<sf::Vector2i> getStartSquare() const { return m_startSquare; }

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(m_card, states);
	}

private:
	inline static const sf::Vector2f retreatTarget = { 1125.f, 200.f };
	inline static const float retreatSpeed = 1000.f;

private:
	Card m_card;
	bool m_isRetreating = false;
	std::optional<sf::Vector2i> m_startSquare;
};