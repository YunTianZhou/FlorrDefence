#pragma once
#include <string>
#include <functional>
#include <SFML/Graphics.hpp>
#include "RoundRect.hpp"

class RawButton : public sf::Transformable {
public:
	RawButton() = default;

	virtual void setSize(sf::Vector2f size);
	virtual void setRect(sf::FloatRect rect);
	void setDisabled(bool disabled);

	void update(sf::Vector2f mousePos);
	void onMouseButtonPressed(sf::Event::MouseButtonPressed event);
	bool onMouseButtonReleased(sf::Event::MouseButtonReleased event);

	sf::FloatRect getRect() const { return sf::FloatRect(getPosition(), m_size); }
	bool isHovered() const { return m_hovered; };
	bool isHeld() const { return m_held; };
	bool isDisabled() const { return m_disabled; };

protected:
	virtual void onUpdate() {};
	virtual bool onClick() { return true; };

protected:
	sf::Vector2f m_size;
	bool m_hovered = false;
	bool m_held = false;
	bool m_disabled = false;
};

class Button : public RawButton, public sf::Drawable {
public:
	Button();

	void setSize(sf::Vector2f size) override;
	void setRect(sf::FloatRect rect) override;

	void setFillColor(sf::Color color);
	void setOutline(sf::Color color, float thickness);
	void setString(const std::string& text);
	void setCharactorSize(unsigned int size);
	void setTextOffset(sf::Vector2f offset);

	sf::FloatRect getTextBounds() const { return m_text.getGlobalBounds(); }
	RawButton getRaw() { return *this; }

private:
	void updateText() const;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

protected:
	sf::Color m_fillColor;
	sf::Color m_outlineColor;
	sf::Color m_actualFillColor;
	sf::Vector2f m_textOffset;
	mutable sf::RoundRect m_box;
	mutable sf::Text m_text;
	mutable bool m_textUpdated = false;
};

class ClickButton : public Button {
public:
	ClickButton() = default;

private:
	void onUpdate() override;
	bool onClick() override;
};

class RadioButton : public Button {
public:
	RadioButton() = default;

	void setIdentifier(const std::string& id) { m_identifier = id; }
	void setVar(std::string& var) { m_varPointer = &var; }
	bool isSelected() const { return *m_varPointer == m_identifier; }

private:
	void onUpdate() override;
	bool onClick() override;

private:
	std::string m_identifier;
	std::string* m_varPointer = nullptr;
};

class RadioButtonGroup : public sf::Drawable {
public:
	RadioButtonGroup() = default;

	void addButton(RadioButton button);
	void update(sf::Vector2f mousePos);
	void onMouseButtonPressed(sf::Event::MouseButtonPressed event);
	bool onMouseButtonReleased(sf::Event::MouseButtonReleased event);

	void setVar(const std::string& var) { m_sharedVar = var; }
	const std::string& getVar() const { return m_sharedVar; }

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	std::vector<RadioButton> m_buttons;
	std::string m_sharedVar;
};
