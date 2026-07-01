#pragma once
#include <string>
#include <map>
#include <set>
#include <vector>
#include <functional>
#include <stdexcept>
#include <SFML/Graphics.hpp>

class Buff {
public:
	using Func = std::function<float(float, float)>;

	inline static const Func Min = [](float a, float b) { return a < b ? a : b; };
	inline static const Func Max = [](float a, float b) { return a > b ? a : b; };
	inline static const Func Add = [](float a, float b) { return a + b; };
	inline static const Func Mul = [](float a, float b) { return a * b; };
	inline static const Func AddFactor = [](float a, float b) { return a + a * b; };
	inline static const Func AddFactor2 = [](float a, float b) { return a + (a + 1) * b; };
	inline static const Func AddFactor3 = [](float a, float b) { return a + (1 - a) * b; };
	inline static const Func SubFactor = [](float a, float b) { return a - a * b; };

public:
	Buff(const Func& addFunc, const Func& applyFunc)
		: m_addFunc(addFunc), m_applyFunc(applyFunc) {
	}

	void add(float value) { m_value = m_addFunc(m_value, value); }

	float apply(float value) const { return m_applyFunc(value, m_value); }

	void set(float value = 0.f) { m_value = value; }

	float get() const { return m_value; }

private:
	float m_value = 0.f;
	const Func& m_addFunc;
	const Func& m_applyFunc;
};

struct BuffGroup {
	Buff speed;
	Buff bodyDamage;
	Buff healValue;
	Buff overheal;
	Buff reload;
	Buff damage;
	Buff summoner;
	Buff reach;
	Buff luck;
	Buff health;
	Buff evasion;
	Buff damage_reduction;
	Buff mob_spawn_rate;
	Buff shop;
	Buff antennae;
	Buff yin_yang;

	std::unordered_map<std::string, Buff*> buffs;

	BuffGroup()
		: speed(Buff::Add, Buff::AddFactor),
		  bodyDamage(Buff::Add, Buff::Add),
		  healValue(Buff::Add, Buff::Add),
		  overheal(Buff::Add, Buff::Mul),
		  reload(Buff::Min, Buff::AddFactor),
		  damage(Buff::Add, Buff::AddFactor),
		  summoner(Buff::Add, Buff::AddFactor),
		  reach(Buff::Add, Buff::AddFactor),
		  luck(Buff::Add, Buff::Add),
		  health(Buff::Add, Buff::AddFactor),
		  evasion(Buff::Add, Buff::Mul),
		  damage_reduction(Buff::Add, Buff::SubFactor),
		  mob_spawn_rate(Buff::Add, Buff::AddFactor),
		  shop(Buff::Max, Buff::Max),
		  antennae(Buff::Max, Buff::Max),
		  yin_yang(Buff::Max, Buff::Max)
	{
		buffs = {
			{"speed", &speed},
			{"body_damage", &bodyDamage},
			{"heal_value", &healValue},
			{"overheal", &overheal},
			{"reload", &reload},
			{"damage", &damage},
			{"summoner", &summoner},
			{"reach", &reach},
			{"luck", &luck},
			{"health", &health},
			{"evasion", &evasion},
			{"damage_reduction", &damage_reduction},
			{"mob_spawn_rate", &mob_spawn_rate},
			{"shop", &shop},
			{"antennae", &antennae},
			{"yin_yang", &yin_yang}
		};
	}

	Buff& get(const std::string& name) {
		auto it = buffs.find(name);
		if (it == buffs.end())
			throw std::runtime_error("Unknown buff name: " + name);
		return *it->second;
	}

	const Buff& get(const std::string& name) const {
		auto it = buffs.find(name);
		if (it == buffs.end())
			throw std::runtime_error("Unknown buff name: " + name);
		return *it->second;
	}

	void reset() {
		for (auto& [_, buff] : buffs)
			buff->set();
	}

	void mergeFrom(const BuffGroup& group1, const BuffGroup& group2) {
		for (auto& [name, buff] : buffs) {
			buff->set(group1.get(name).get());
			buff->add(group2.get(name).get());
		}
		reload.set(Buff::AddFactor2(group1.reload.get(), group2.reload.get()));
		evasion.set(Buff::AddFactor3(group1.evasion.get(), group2.evasion.get()));
		damage_reduction.set(Buff::AddFactor3(group1.damage_reduction.get(), group2.damage_reduction.get()));
	}
};

struct BuffManager {
public:
	struct Entry {
		std::string rarity;
		sf::Vector2i square;
	};

	void setAntennaeLevel(int antennaeLevel) {
		m_antennaeLevel = antennaeLevel;
		m_cards.clear();
	}

	void add(const CardInfo& card, sf::Vector2i square) {
		if (flowerBuffCards.find(card.type) != flowerBuffCards.end() &&
			RARITIE_LEVELS.at(card.rarity) > m_antennaeLevel)
			return;

		auto it = m_cards.find(card.type);
		if (it == m_cards.end() ||
			RARITIE_LEVELS.at(card.rarity) > RARITIE_LEVELS.at(it->second.rarity)) {
			m_cards[card.type] = Entry{ card.rarity, square };
		}
	}

	bool find(const CardInfo& card, sf::Vector2i square) const {
		auto it = m_cards.find(card.type);
		if (it == m_cards.end())
			return false;
		return it->second.square == square;
	}

	std::vector<sf::Vector2i> getSquares() const {
		std::vector<sf::Vector2i> squares;
		squares.reserve(m_cards.size());
		for (const auto& [_, entry] : m_cards)
			squares.push_back(entry.square);
		return squares;
	}

public:
	inline static const std::set<std::string> flowerBuffCards = {
		"cutter", "leaf", "rose", "amulet", "talisman", "disc", "shell"
	};

private:
	int m_antennaeLevel = 0;
	std::unordered_map<std::string, Entry> m_cards;
};
