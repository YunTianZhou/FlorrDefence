#include "TalentDescription.hpp"

#include <fstream>
#include "AssetManager.hpp"
#include "Tools.hpp"

TalentDescription::TalentDescription() :
	m_title(AssetManager::getFont()),
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

	m_content.setCharacterSize(contentCharSize);
	m_content.setOutlineColor(sf::Color::Black);
	m_content.setOutlineThickness(contentCharSize * 0.05f);

	loadData();
}

void TalentDescription::set(const CardInfo& talent, sf::Vector2f talentCenter, float talentRadius) {
	if (m_talent != talent) {
		m_talent = talent;

		updateText();
		updateTextPosition();
	}

	if (m_talentCenter != talentCenter || m_talentRadius != talentRadius) {
		m_talentCenter = talentCenter;
		m_talentRadius = talentRadius;

		updateBackgroundPosition();
	}

	m_isVerified = true;
}

void TalentDescription::loadData() {
	std::ifstream ifs("res/config/talent_description.json");

	if (!ifs.is_open())
		throw std::runtime_error("Failed to open talent_description.json");

	ifs >> m_data;

	// Color table
	m_colorTable.clear();

	for (auto& [color, j] : m_data["colors"].items()) {
		unsigned char r = (unsigned char)(j.at(0).get<int>());
		unsigned char g = (unsigned char)(j.at(1).get<int>());
		unsigned char b = (unsigned char)(j.at(2).get<int>());

		m_colorTable[color] = { r, g, b };
	}
}


void TalentDescription::updateText() {
	if (!m_data["talents"].contains(m_talent.type))
		return;

	const nlohmann::json& j = m_data["talents"][m_talent.type];

	// Title
	m_title.setString(capitalized(m_talent.type));

	// Rarity
	m_rarity.setString(capitalized(m_talent.rarity));
	m_rarity.setFillColor(LIGHT_COLORS.at(m_talent.rarity));

	// Content
	std::string description = j.value("description", "Undefined");
	std::string description_formated;

	std::string id = std::format("{}_{}", m_talent.type, m_talent.rarity);
	const auto& attrib = TALENT_ATTRIBS.at(TALENT_ID_TO_INDEX.at(id));
	float value = attrib.buff_value;

	float prevValue = 0;
	if (attrib.prev_id.has_value()) {
		const std::string& prevId = *attrib.prev_id;
		prevValue = TALENT_ATTRIBS.at(TALENT_ID_TO_INDEX.at(prevId)).buff_value;
	}

	std::string valueType = j.value("value_type", "Undefined");
	sf::Color currentColor = sf::Color::White;
	std::string buffer;

	auto flush = [&]() {
		if (buffer.empty())
			return;

		m_content << currentColor << sf::String(buffer);
		buffer.clear();
	};

	auto parse = [&](float value) {
		if (valueType == "add_percent") {
			m_content << sf::String(toPercent(value + 1.f));
		}
		else if (valueType == "rarity") {
			std::string rarity = RARITIES[(int)round(value) - 1];
			m_content << LIGHT_COLORS.at(rarity) << sf::String(capitalized(rarity)) << currentColor;
		}
		else
			throw std::runtime_error(std::format("Unknown value type '{}'", valueType));
	};

	m_content.clear();
	m_content << sf::Text::Regular << currentColor;

	for (size_t i = 0; i < description.size();) {
		// value
		if (description[i] == '$') {
			flush();
			parse(value);
			i++;
			continue;
		}

		// prev value
		else if (description[i] == '#') {
			flush();
			parse(prevValue);
			i++;
			continue;
		}

		// </>
		else if (description.compare(i, 3, "</>") == 0) {
			flush();
			currentColor = sf::Color::White;
			i += 3;
			continue;
		}

		// <color>
		else if (description[i] == '<') {
			size_t end = description.find('>', i);
			if (end != std::string::npos)
			{
				flush();

				std::string colorName = description.substr(i + 1, end - i - 1);

				auto it = m_colorTable.find(colorName);
				if (it != m_colorTable.end())
					currentColor = it->second;
				else
					currentColor = sf::Color::White;

				i = end + 1;
				continue;
			}
		}

		buffer += description[i];
		i++;
	}

	flush();
}

void TalentDescription::updateTextPosition() {
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

	// Background size
	m_size = { std::max(minWidth, width), y + bottomPadding };
	m_background.setSize(m_size);
}

void TalentDescription::updateBackgroundPosition() {
	float delta = m_talentRadius + outsidePadding;

	float y = m_talentCenter.y - delta - m_size.y > outsidePadding ?
			  m_talentCenter.y - delta - m_size.y : m_talentCenter.y + delta;
	float x = m_talentCenter.x - m_size.x / 2;
	x = std::clamp(x, outsidePadding, VIEW_SIZE.x - m_size.x - outsidePadding);

	m_position = { x, y };
}

void TalentDescription::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform.translate(m_position);

	target.draw(m_background, states);
	target.draw(m_title, states);
	target.draw(m_rarity, states);
	target.draw(m_content, states);
}
