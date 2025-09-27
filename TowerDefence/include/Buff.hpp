#pragma once
#include <string>
#include <map>
#include <SFML/Graphics.hpp>

class AddBuff {
public:
	void add(float value) {
		m_value += value;
	}

	float apply(float value) const {
		return value + m_value;
	}

private:
	float m_value = 0.f;
};

class AddFactorBuff {
public:
	void add(float factor) {
		m_factor = m_factor + factor;
	}

	float apply(float value) const {
		return value + value * m_factor;
	}

private:
	float m_factor = 0.f;
};

class FactorBuff {
public:
	void add(float factor) {
		m_factor = m_factor + factor;
	}

	float apply(float value) const {
		return value * m_factor;
	}

private:
	float m_factor = 0.f;
};

struct Buff {
	AddFactorBuff speed;
	AddBuff bodyDamage;
	AddBuff heal;
	FactorBuff overheal;
	AddFactorBuff reload;

	void add(const std::string& name, float value) {
		if (name == "speed_buff")
			speed.add(value);
		else if (name == "body_damage_buff")
			bodyDamage.add(value);
		else if (name == "heal_buff")
			heal.add(value);
		else if (name == "overheal_buff")
			overheal.add(value);
		else if (name == "relaod_buff")
			reload.add(value);
		else
			throw;
	}

	float apply(const std::string& name, float value) {
		if (name == "speed")
			return speed.apply(value);
		else if (name == "body_damage")
			return bodyDamage.apply(value);
		else if (name == "heal")
			return heal.apply(value);
		else if (name == "overheal")
			return overheal.apply(value);
		else if (name == "reload")
			return reload.apply(value);
		else
			throw;
	}
};

struct BuffManager {
public:
	struct Entry {
		std::string rarity;
		sf::Vector2i square;
	};

	void add(CardInfo card, sf::Vector2i square) {
		if (flowerBuffCards.find(card.type) != flowerBuffCards.end() &&
			RARITIE_LEVELS.at(card.rarity) > getAntennaeLevel())
			return;
		auto it = m_cards.find(card.type);
		if (it == m_cards.end() || RARITIE_LEVELS.at(card.rarity) > RARITIE_LEVELS.at(it->second.rarity)) {
			m_cards[card.type] = Entry(card.rarity, square);
		}
	}

	int getAntennaeLevel() {
		auto it = m_cards.find("antennae");
		if (it == m_cards.end())
			return 0;
		return RARITIE_LEVELS.at(it->second.rarity);
	}

	bool find(CardInfo card, sf::Vector2i square) {
		auto it = m_cards.find(card.type);
		if (it == m_cards.end()) {
			return false;
		}
		return it->second.square == square;
	}

	std::vector<sf::Vector2i> get_squares() const {
		std::vector<sf::Vector2i> squares;
		for (const auto& [_, entry] : m_cards) {
			squares.push_back(entry.square);
		}
		return squares;
	}

public:
	inline static const std::set<std::string> flowerBuffCards = {
		"cutter", "leaf", "rose", "amulet"
	};

private:
	std::map<std::string, Entry> m_cards;
};
