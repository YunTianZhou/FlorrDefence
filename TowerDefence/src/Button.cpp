#include "Button.hpp"
#include "AssetManager.hpp"

static sf::Color lightenColor(const sf::Color& color, float amount = 0.15f) {
	auto lighten = [&](unsigned int c) {
		return (unsigned int)(std::min(255.f, c + (255 - c) * amount));
		};
	return sf::Color(lighten(color.r), lighten(color.g), lighten(color.b), color.a);
}

static sf::Color darkenColor(const sf::Color& color, float amount = 0.15f) {
	auto darken = [&](unsigned int c) {
		return (unsigned int)(std::max(0.f, c * (1.f - amount)));
		};
	return sf::Color(darken(color.r), darken(color.g), darken(color.b), color.a);
}

void RawButton::setSize(sf::Vector2f size) {
	m_size = size;
}

void RawButton::setRect(sf::FloatRect rect) {
	setPosition(rect.position);
	setSize(rect.size);
}

void RawButton::setDisabled(bool disabled) {
	if (m_disabled = disabled) {
		m_hovered = false;
		m_held = false;
	}
}

void RawButton::update(sf::Vector2f mousePos) {
	m_hovered = !m_disabled && getRect().contains(mousePos);
	m_held &= m_hovered;
	onUpdate();
}

void RawButton::onMouseButtonPressed(sf::Event::MouseButtonPressed event) {
	if (event.button == sf::Mouse::Button::Left && m_hovered) {
		m_held = true;
	}
}

bool RawButton::onMouseButtonReleased(sf::Event::MouseButtonReleased event) {
	if (event.button == sf::Mouse::Button::Left && m_held) {
		m_held = false;
		return onClick();
	}
	return false;
}

Button::Button()
	: m_text(AssetManager::getFont()) {
	m_text.setFillColor(sf::Color::White);
	m_text.setOutlineColor(sf::Color::Black);
}

void Button::setSize(sf::Vector2f size) {
	RawButton::setSize(size);

	m_box.setSize(size);

	float minDim = std::min(size.x, size.y);
	float radius = std::clamp(minDim * 0.15f, 2.f, 20.f);
	m_box.setRadius(radius);
}

void Button::setRect(sf::FloatRect rect) {
	setPosition(rect.position);
	setSize(rect.size);
}

void Button::setFillColor(sf::Color color) {
	m_fillColor = color;
	m_actualFillColor = color;
}

void Button::setOutline(sf::Color color, float thickness) {
	m_outlineColor = color;
	m_box.setOutlineThickness(thickness);

	m_textUpdated = false;
}

void Button::setString(const std::string& text) {
	m_text.setString(text);
	m_textUpdated = false;
}

void Button::setCharactorSize(unsigned int charSize) {
	m_text.setCharacterSize(charSize);
	m_text.setOutlineThickness(0.0625f * charSize);
	m_textUpdated = false;
}

void Button::setTextOffset(sf::Vector2f offset) {
	m_textOffset = offset;
	m_textUpdated = false;
}

void Button::updateText() const {
	assert(m_box.getSize() != sf::Vector2f());
	m_text.setOrigin(m_text.getGlobalBounds().size / 2.f + m_text.getLocalBounds().position);
	m_text.setPosition(m_box.getSize() / 2.f + m_textOffset);

	m_textUpdated = true;
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!m_textUpdated)
		updateText();

	states.transform *= getTransform();

	if (m_disabled) {
		m_box.setFillColor(LIGHT_COLORS.at("disabled"));
		m_box.setOutlineColor(DARK_COLORS.at("disabled"));
	}
	else {
		m_box.setFillColor(m_actualFillColor);
		m_box.setOutlineColor(m_outlineColor);
	}
	target.draw(m_box, states);
	target.draw(m_text, states);
}

void ClickButton::onUpdate() {
	if (m_held) {
		m_actualFillColor = darkenColor(m_fillColor);
	}
	else if (m_hovered) {
		m_actualFillColor = lightenColor(m_fillColor);
	}
	else {
		m_actualFillColor = m_fillColor;
	}
}

bool ClickButton::onClick() {
	return true;
}

void RadioButton::onUpdate() {
	if (m_held || isSelected()) {
		m_actualFillColor = darkenColor(m_fillColor);
	}
	else if (m_hovered) {
		m_actualFillColor = lightenColor(m_fillColor);
	}
	else {
		m_actualFillColor = m_fillColor;
	}
}

bool RadioButton::onClick() {
	if (!isSelected()) {
		*m_varPointer = m_identifier;
		return true;
	}
	return false;
}

void RadioButtonGroup::addButton(RadioButton button) {
	button.setVar(m_sharedVar);
	m_buttons.push_back(std::move(button));
}

void RadioButtonGroup::update(sf::Vector2f mousePos) {
	for (auto& button : m_buttons) {
		button.update(mousePos);
	}
}

void RadioButtonGroup::onMouseButtonPressed(sf::Event::MouseButtonPressed event) {
	for (auto& button : m_buttons) {
		button.onMouseButtonPressed(event);
	}
}

bool RadioButtonGroup::onMouseButtonReleased(sf::Event::MouseButtonReleased event) {
	bool changed = false;
	for (auto& button : m_buttons) {
		changed |= button.onMouseButtonReleased(event);
	}
	return changed;
}

void RadioButtonGroup::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	for (auto& button : m_buttons) {
		target.draw(button, states);
	}
}
