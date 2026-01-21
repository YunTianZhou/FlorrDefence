#pragma once
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"

class Effect : public sf::Drawable {
public:
	Effect(const SharedInfo& info);

	virtual void update() = 0;
	virtual bool isDone() = 0;

private:
	const SharedInfo& m_info;

};

class LightningEffect : public Effect {
public:
	LightningEffect(const SharedInfo& info, sf::Vector2f root, int rootConnected, std::vector<sf::Vector2f> positions);

	void update();
	bool isDone();

private:
	void buildLine(sf::Vector2f start, sf::Vector2f end);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	inline static const float thinkness = 1.f;
	inline static const float duration = 0.3f;

private:
	std::vector<sf::RectangleShape> m_lines;
	sf::Clock m_clock;
};
