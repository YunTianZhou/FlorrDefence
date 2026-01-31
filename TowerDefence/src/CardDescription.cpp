#include "CardDescription.hpp"

#include <fstream>
#include "AssetManager.hpp"
#include "Tools.hpp"

LabelEntry::LabelEntry(
	const std::string& labelText,
	const std::string& valueText,
	unsigned int charSize,
	sf::Color labelColor,
	sf::Color valueColor
) : label(AssetManager::getFont()), value(AssetManager::getFont()) {
	label.setString(labelText + ":");
	label.setCharacterSize(charSize);
	label.setFillColor(labelColor);
	label.setOutlineColor(sf::Color::Black);
	label.setOutlineThickness(charSize * 0.05f);

	value.setString(" " + valueText);
	value.setCharacterSize(charSize);
	value.setFillColor(valueColor);
	value.setOutlineColor(sf::Color::Black);
	value.setOutlineThickness(charSize * 0.05f);

	updateLayout();
}

void LabelEntry::setPosition(sf::Vector2f pos) {
	label.setPosition(pos);
	updateLayout();
}

void LabelEntry::setValue(const std::string& valueText) {
	value.setString(" " + valueText);
	updateLayout();
}

void LabelEntry::updateLayout() {
	auto bounds = label.getLocalBounds();

	value.setPosition({
		label.getPosition().x + bounds.position.x + bounds.size.x,
		label.getPosition().y
	});
}

void LabelEntry::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();

	target.draw(label, states);
	target.draw(value, states);
}

CardDescription::CardDescription()
	: m_title(AssetManager::getFont()),
	  m_rarity(AssetManager::getFont()),
	  m_content(AssetManager::getFont()),
	  m_lineSpacing(AssetManager::getFont().getLineSpacing(contentCharSize)) {
	m_background.setFillColor({ 0, 0, 0, 128 });
	m_background.setRadius(5.f);

	m_title.setFillColor(sf::Color::White);
	m_title.setOutlineColor(sf::Color::Black);
	m_title.setCharacterSize(titleCharSize);
	m_title.setOutlineThickness(titleCharSize * 0.05f);

	m_rarity.setOutlineColor(sf::Color::Black);
	m_rarity.setCharacterSize(contentCharSize);
	m_rarity.setOutlineThickness(contentCharSize * 0.05f);

	m_content.setFillColor(sf::Color::White);
	m_content.setOutlineColor(sf::Color::Black);
	m_content.setCharacterSize(contentCharSize);
	m_content.setOutlineThickness(contentCharSize * 0.05f);
	m_content.setLineSpacing(contentLineSpacing);

	load_data();
}

void CardDescription::set(const CardInfo& card, sf::Vector2f cardCenter, float cardSize) {
	if (m_card != card) {
		m_card = card;

		updateText();
		updateTextPosition();
	}

	if (m_cardCenter != cardCenter || m_cardSize != cardSize) {
		m_cardCenter = cardCenter;
		m_cardSize = cardSize;

		updateBackgroundPosition();
	}

	m_isVerified = true;
}

void CardDescription::load_data() {
	std::ifstream ifs("res/config/tower_descrption.json");

	if (!ifs.is_open()) 
		throw std::runtime_error("Failed to open tower_descrption.json");

	ifs >> m_info;

	// Color table
	m_colorTable.clear();

	for (auto& [color, j] : m_info["colors"].items()) {
		unsigned char r = (unsigned char)(j.at(0).get<int>());
		unsigned char g = (unsigned char)(j.at(1).get<int>());
		unsigned char b = (unsigned char)(j.at(2).get<int>());

		m_colorTable[color] = { r, g, b };
	}
}

std::string CardDescription::parseAttrib(const std::string& value, const std::string& type) {
	const auto& entry = TOWER_ATTRIBS[m_card.type].rarities[m_card.rarity];
	float attrib = entry.attribs.at(value);

	if (type == "int") {
		return toNiceString((int64_t)attrib);
	}
	else if (type == "sceconds") {
		return formatFloat(std::max(0.1f, attrib), 1) + " seconds";
	}
	else if (type == "per_second") {
		return toNiceString((int64_t)attrib) + "/s";
	}
	else if (type == "range") {
		return formatFloat(attrib, 1) + " squares";
	}
	else if (type == "percent") {
		return toNiceString((int64_t)(attrib * 100)) + "%";
	}
	else {
		throw std::runtime_error(std::format("Unknown value type '{}'", type));
	}
}

void CardDescription::updateText() {
	if (!m_info["cards"].contains(m_card.type))
		return;

	const nlohmann::json& j = m_info["cards"][m_card.type];

	// Title 
	std::string name = j.value("name", "Undefined");
	std::string type = j.value("type", "Undefined");
	m_title.setString(std::format("{} ({})", name, type));

	// Rarity
	m_rarity.setString(capitalized(m_card.rarity));
	m_rarity.setFillColor(LIGHT_COLORS.at(m_card.rarity));

	// Content
	m_content.setString(j.value("description", "Undefined"));

	// Labels
	m_labels.clear();

	if (j.contains("labels")) {
		for (auto& entry : j["labels"]) {
			std::string label = entry.value("label", "Undefined");
			std::string labelColor = entry.value("label_color", "Undefined");
			std::string value = entry.value("value", "Undefined");
			std::string valueColor = entry.value("value_color", "white");
			std::string valueType = entry.value("value_type", "text");

			if (valueType == "text") {

			}
			else if (valueType == "rarity") {
				value = capitalized(m_card.rarity);
				valueColor = m_card.rarity;
			}
			else {
				value = parseAttrib(value, valueType);
			}
			
			m_labels.emplace_back(
				label, value,
				contentCharSize,
				m_colorTable.at(labelColor),
				m_colorTable.at(valueColor)
			);
		}
	}
}

void CardDescription::updateTextPosition() {
	float y = topPadding, width = 0.f;

	auto updateBound = [&](const sf::FloatRect& bound) {
		width = std::max(width, bound.position.x + bound.size.x + rightPadding);
		y = bound.position.y + bound.size.y;
	};

	// Title 
	m_title.setPosition({ leftPadding, y });
	updateBound(m_title.getGlobalBounds());

	// Rarity
	y += rarityInterval;
	m_rarity.setPosition({ leftPadding, y });
	updateBound(m_rarity.getGlobalBounds());

	// Content
	y += contentInterval;
	m_content.setPosition({ leftPadding, y });
	updateBound(m_content.getGlobalBounds());

	// Labels
	y += labelInterval;
	for (LabelEntry& entry : m_labels) {
		entry.setPosition({ leftPadding, y });
		y += m_lineSpacing * contentLineSpacing;
	}

	// Background size
	m_size = { std::max(minWidth, width), y + bottomPadding };
	m_background.setSize(m_size);
}

void CardDescription::updateBackgroundPosition() {
	float delta = m_cardSize / 2 + outsidePadding;

	float y = m_cardCenter.y - delta - m_size.y > outsidePadding ?
		      m_cardCenter.y - delta - m_size.y : m_cardCenter.y + delta;
	float x = m_cardCenter.x - m_size.x / 2;
	x = std::clamp(x, outsidePadding, VIEW_SIZE.x - m_size.x - outsidePadding);

	m_position = { x, y };
}

void CardDescription::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform.translate(m_position);
	
	target.draw(m_background, states);
	target.draw(m_title, states);
	target.draw(m_rarity, states);
	target.draw(m_content, states);

	for (const LabelEntry& label : m_labels)
		target.draw(label, states);
}
