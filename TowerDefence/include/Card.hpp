#pragma once
#include <SFML/Graphics.hpp>
#include "RoundRect.hpp"
#include "Constants.hpp"

class Card : public sf::Drawable, public sf::Transformable {
public:
	Card();

	void setCard(const CardInfo& card);
	virtual void setLength(float length);
	void setDisabled(bool disability);
	const float getLength() const { return m_length; }
	sf::FloatRect getRect() const { return sf::FloatRect(getPosition(), m_backgroundRect.getSize()); };
	const CardInfo& getCard() const { return m_card; };
	bool isDisabled() const { return m_disabled; }

protected:
	void updateColor();
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

protected:
	CardInfo m_card;
	sf::RoundRect m_backgroundRect;
	sf::RectangleShape m_texRect;
	float m_length = 0.f;
	float m_reload = 0.f;
	bool m_disabled = false;
};

class CardStack : public Card {
public:
	CardStack();

	void setLength(float length) override;
	void setCount(int count);
	void setInfo(const CardStackInfo& info);
	int getCount() const { return m_count; }
	CardStackInfo getInfo() const { return { m_card, m_count }; }

private:
	void updateText() const;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	int m_count = 0;
	mutable sf::Text m_text;
	mutable bool m_textUpdated = false;
};

class TowerCard : public Card {
public:
	void setReload(float reload, bool top);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	float m_reload = 1.f;
	bool m_top = false;
	mutable sf::RectangleShape m_reloadRect;
};
