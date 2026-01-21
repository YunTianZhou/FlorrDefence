#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <SFML/Graphics.hpp>

inline const sf::Vector2f VIEW_SIZE(1700.f, 1100.f);
inline const sf::Vector2u WINDOW_INIT_SIZE(850u, 550u);

struct CardInfo {
	std::string rarity;
	std::string type;

	bool operator==(const CardInfo& other) const {
		return rarity == other.rarity && type == other.type;
	}

	bool operator<(const CardInfo& other) const {
		return std::tie(rarity, type) < std::tie(other.rarity, other.type);
	}
};

using MobInfo = CardInfo;

struct CardStackInfo {
	CardInfo card;
	int count;

	bool operator==(const CardStackInfo& other) const {
		return card == other.card && count == other.count;
	}
};

enum class DamageType {
	Normal,
	Lightning
};

DamageType stringToDamageType(const std::string& str);

struct TowerAttribs {
	struct RarityEntry {
		int64_t price = 0;
		std::map<std::string, float> attribs;
	};
	std::string type;
	DamageType damageType = DamageType::Normal;
	std::map<std::string, RarityEntry> rarities;

	const RarityEntry& operator[](const std::string& rarity) const {
		return rarities.at(rarity);
	}
};

struct MobAttribs {
	struct RarityEntry {
		int hp = 0;
		float speed = 0;
		int damage = 0;
		int armor = 0;
		int64_t coinDrop = 0;
		int xpDrop = 0;
		std::map<std::string, float> attribs;
	};
	std::map<std::string, RarityEntry> rarities;

	const RarityEntry& operator[](const std::string& rarity) const {
		return rarities.at(rarity);
	}
};

struct TalentAttribs {
	std::string id;
	std::string type;
	std::string rarity;
	std::optional<std::string> prev_id;
	sf::Vector2f position;
	std::string buff_type;
	float buff_value = 0;
	int xp_cost = 0;
};

struct InitStates {
	int hp = 0;
	int xp = 0;
	int bodyDamage = 0;
	int level = 0;
	int64_t coin = 0;
	int talent = 0;
	std::vector<CardStackInfo> cards;
};

struct TimeRange {
	sf::Time lower;
	sf::Time upper;
};

const int INF = 1 << 30;
const sf::Time TICK = sf::milliseconds(125);

extern const std::vector<std::string> RARITIES;
extern const std::map<std::string, int> RARITIE_LEVELS;

extern const std::vector<std::string> SHOP_TYPES;
extern const std::map<std::string, int> SHOP_PRODUCT_COUNTS;
extern const std::map<std::string, sf::Time> SHOP_REFRESH_TIMES;

extern const std::vector<std::string> TOWER_TYPES;
extern const std::map<CardInfo, MobInfo> TOWER_SUMMON_MOBS;
extern const std::set<std::string> LIGHTNING_TOWERS;

extern const std::map<std::string, float> MOB_RARITY_SCALES;

extern const std::map<std::string, float> CRAFT_PROBS;
extern const std::map<std::string, TimeRange> CRAFT_TIME_RANGES;

extern const std::vector<sf::Vector2i> PATH_SQUARES;

extern const std::map<std::string, sf::Color> LIGHT_COLORS;
extern const std::map<std::string, sf::Color> DARK_COLORS;

extern InitStates INIT_STATES;
extern std::map<std::string, TowerAttribs> TOWER_ATTRIBS;
extern std::map<std::string, MobAttribs> MOB_ATTRIBS;
extern std::vector<TalentAttribs> TALENT_ATTRIBS;
extern std::unordered_map<std::string, int> TALENT_ID_TO_INDEX;

void loadConstants();
