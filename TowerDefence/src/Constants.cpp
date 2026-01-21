#include <cassert>
#include <fstream>
#include <chrono>
#include <nlohmann/json.hpp>
#include "Constants.hpp"

const std::vector<std::string> RARITIES = {
	"common", "unusual", "rare", "epic", "legendary",
	"mythic", "ultra", "super", "unique"
};

const std::map<std::string, int> RARITIE_LEVELS = {
	{"common", 1},
	{"unusual", 2},
	{"rare", 3},
	{"epic", 4},
	{"legendary", 5},
	{"mythic", 6},
	{"ultra", 7},
	{"super", 8},
	{"unique", 9}
};

const std::vector<std::string> SHOP_TYPES = {
	"common", "rare", "legendary", "ultra", "unique"
};

const std::map<std::string, int> SHOP_PRODUCT_COUNTS = {
	{"common", 20}, {"rare", 15}, {"legendary", 10}, {"ultra", 5}, {"unique", 3}
};

const std::map<std::string, sf::Time> SHOP_REFRESH_TIMES = {
	{"common", sf::seconds(120)},
	{"rare", sf::seconds(180)},
	{"legendary", sf::seconds(300)},
	{"ultra", sf::seconds(600)},
	{"unique", sf::seconds(1200)}
};

const std::vector<std::string> TOWER_TYPES = {
	"amulet", "ant_egg", "antennae", "basic", "beetle_egg", 
	"bone", "bur", "chip", "coin", "corn", 
	"cutter", "dahlia", "dice", "faster", "golden_leaf", 
	"jelly", "laser", "leaf", "light", "lightning", 
	"missile", "pincer", "pollen", "rice", "rock", 
	"rose", "shovel", "stinger", "triangle", "web"
};

const std::map<CardInfo, MobInfo> TOWER_SUMMON_MOBS = {
	// Ant Egg
	{ {"common", "ant_egg" }, { "common", "ant_soldier" } },
	{ {"unusual", "ant_egg" }, { "unusual", "ant_soldier" } },
	{ {"rare", "ant_egg" }, { "rare", "ant_soldier" } },
	{ {"epic", "ant_egg" }, { "epic", "ant_soldier" } },
	{ {"legendary", "ant_egg" }, { "legendary", "ant_soldier" } },
	{ {"mythic", "ant_egg" }, { "mythic", "ant_soldier" } },
	{ {"ultra", "ant_egg" }, { "mythic", "ant_soldier" } },
	{ {"super", "ant_egg" }, { "ultra", "ant_soldier" } },
	{ {"unique", "ant_egg" }, { "ultra", "ant_soldier" } },

	// Beetle Egg
	{ {"common", "beetle_egg" }, { "common", "beetle" } },
	{ {"unusual", "beetle_egg" }, { "unusual", "beetle" } },
	{ {"rare", "beetle_egg" }, { "rare", "beetle" } },
	{ {"epic", "beetle_egg" }, { "epic", "beetle" } },
	{ {"legendary", "beetle_egg" }, { "legendary", "beetle" } },
	{ {"mythic", "beetle_egg" }, { "mythic", "beetle" } },
	{ {"ultra", "beetle_egg" }, { "mythic", "beetle" } },
	{ {"super", "beetle_egg" }, { "ultra", "beetle" } },
	{ {"unique", "beetle_egg" }, { "ultra", "beetle" } },
};

const std::set<std::string> LIGHTNING_TOWERS = {
	"lightning", "laser"
};

const std::map<std::string, float> MOB_RARITY_SCALES = {
	{"common", 0.12f},
	{"unusual", 0.2f},
	{"rare", 0.28f},
	{"epic", 0.4f},
	{"legendary", 0.5f},
	{"mythic", 0.6f},
	{"ultra", 0.76f},
	{"super", 1.f},
};

const std::map<std::string, float> CRAFT_PROBS = {  // curr => next
	{"common", 0.64f},
	{"unusual", 0.32f},
	{"rare", 0.16f},
	{"epic", 0.08f},
	{"legendary", 0.04f},
	{"mythic", 0.02f},
	{"ultra", 0.01f},
};

const std::map<std::string, TimeRange> CRAFT_TIME_RANGES = {
	{"common", { sf::seconds(0.1f), sf::seconds(1.f) }},
	{"unusual", { sf::seconds(0.5f), sf::seconds(3.f) }},
	{"rare", { sf::seconds(1.f), sf::seconds(5.f) }},
	{"epic", { sf::seconds(1.f), sf::seconds(8.f) }},
	{"legendary", { sf::seconds(2.f), sf::seconds(10.f) }},
	{"mythic", { sf::seconds(5.f), sf::seconds(20.f) }},
	{"ultra", { sf::seconds(10.f), sf::seconds(30.f) }}
};

const std::vector<sf::Vector2i> PATH_SQUARES = {
	{5, 0}, {5, 1}, {5, 2}, {4, 2}, {3, 2}, {2, 2}, {1, 2}, {1, 3}, {1, 4}, {2, 4}, 
	{3, 4}, {4, 4}, {5, 4}, {6, 4}, {7, 4}, {7, 3}, {7, 2}, {7, 1}, {8, 1}, {9, 1}, 
	{9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8}, {8, 8}, {7, 8}, {6, 8}, 
	{6, 7}, {6, 6}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {2, 7}, {2, 8}, {2, 9}
};

const std::map<std::string, sf::Color> LIGHT_COLORS = {
	{"wood", { 219, 157, 90 } },
	{"gold", { 252, 223, 3 } },
	{"disabled", { 119, 119, 119 } },

	// Rarities
	{"common", { 126, 239, 109 } },
	{"unusual", { 255, 230, 93 } },
	{"rare", { 77, 82, 227 } },
	{"epic", { 134, 31, 222 } },
	{"legendary", { 222, 31, 31 } },
	{"mythic", { 31, 219, 222 } },
	{"ultra", { 255, 43, 117 } },
	{"super", { 43, 255, 163 } },
	{"unique", { 85, 85, 85 } }
};

const std::map<std::string, sf::Color> DARK_COLORS = {
	{"wood", { 177, 127, 73 } },
	{"gold", { 204, 181, 2 } },
	{"disabled", { 96, 96, 96 } },

	// Rarities
	{"common", { 102, 194, 88 } },
	{"unusual", { 207, 186, 75 } },
	{"rare", { 62, 66, 184 } },
	{"epic", { 109, 25, 180 } },
	{"legendary", { 180, 25, 25 } },
	{"mythic", { 25, 177, 180 } },
	{"ultra", { 207, 35, 95 } },
	{"super", { 35, 207, 132 } },
	{"unique", { 69, 69, 69 } }
};

InitStates INIT_STATES;
std::map<std::string, TowerAttribs> TOWER_ATTRIBS;
std::map<std::string, MobAttribs> MOB_ATTRIBS;
std::vector<TalentAttribs> TALENT_ATTRIBS;
std::unordered_map<std::string, int> TALENT_ID_TO_INDEX;

DamageType stringToDamageType(const std::string& str) {
	if (str == "normal")
		return DamageType::Normal;
	else if (str == "lightning")
		return DamageType::Lightning;
	throw std::runtime_error(std::format("invalid damage type '{}'", str));
}

static void loadInitSupplies() {
	std::ifstream ifs("res/config/init_states.json");
	assert(ifs.is_open());

	nlohmann::json j;
	ifs >> j;

	INIT_STATES.hp = j["hp"].get<int>();
	INIT_STATES.xp = j["xp"].get<int>();
	INIT_STATES.bodyDamage = j["body_damage"].get<int>();
	INIT_STATES.level = j["level"].get<int>();
	INIT_STATES.coin = j["coin"].get<int64_t>();
	INIT_STATES.talent = j["talent"].get<int>();
	for (auto& entry : j["cards"])
		INIT_STATES.cards.emplace_back(CardInfo(entry[0], entry[1]), entry[2].get<int>());
}

static void loadTowerAttribs() {
	std::ifstream ifs("res/config/tower_attribs.json");
	assert(ifs.is_open());

	nlohmann::json j;
	ifs >> j;

	for (auto& [type, obj] : j.items()) {
		TowerAttribs ta;
		ta.type = obj["type"].get<std::string>();

		if (obj.find("damage_type") != obj.end()) {
			std::string damageType = obj["damage_type"];
			ta.damageType = stringToDamageType(damageType);
		}
		else {
			ta.damageType = DamageType::Normal;
		}

		for (auto& [rarity, entry] : obj["rarities"].items()) {
			TowerAttribs::RarityEntry& e = ta.rarities[rarity];
			e.price = entry["price"].get<int>();
			for (auto& [key, val] : entry["attribs"].items()) {
				e.attribs[key] = val.get<float>();
			}
		}
		TOWER_ATTRIBS[type] = std::move(ta);
	}

	// TEST
	for (auto& type : TOWER_TYPES) {
		if (TOWER_ATTRIBS.count(type) == 0)
			TOWER_ATTRIBS[type] = {};
		for (auto& rarity : RARITIES) {
			if (TOWER_ATTRIBS[type].rarities.count(rarity) == 0)
				TOWER_ATTRIBS[type].rarities[rarity] = {};
		}
	}
}

static void loadTalentAttribs() {
	std::ifstream ifs("res/config/talent_attribs.json");
	assert(ifs.is_open());

	nlohmann::json j;
	ifs >> j;

	int index = 0;
	for (const auto& entry : j) {
		TalentAttribs t;
		t.id = entry["id"].get<std::string>();
		t.type = entry["type"].get<std::string>();
		t.rarity = entry["rarity"].get<std::string>();
		if (entry.contains("prev_id") && !entry["prev_id"].is_null())
			t.prev_id = entry["prev_id"].get<std::string>();
		else
			t.prev_id = std::nullopt;
		t.position.x = entry["x"].get<float>();
		t.position.y = entry["y"].get<float>();
		t.buff_type = entry["buff_type"].get<std::string>();
		t.buff_value = entry["buff_value"].get<float>();
		t.xp_cost = entry["xp_cost"].get<int>();
		TALENT_ATTRIBS.emplace_back(t);
		TALENT_ID_TO_INDEX[t.id] = index;
		index++;
	}
}

void loadMobAttribs() {
	std::ifstream ifs("res/config/mob_attribs.json");
	assert(ifs.is_open());

	nlohmann::json j;
	ifs >> j;

	for (auto& [tyoe, obj] : j.items()) {
		MobAttribs ta;
		for (auto& [rarity, entry] : obj["rarities"].items()) {
			MobAttribs::RarityEntry& e = ta.rarities[rarity];
			e.hp = entry["hp"].get<int>();
			e.speed = entry["speed"].get<float>();
			e.damage = entry["damage"].get<int>();
			e.armor = entry["armor"].get<int>();
			e.coinDrop = entry["coin_drop"].get<int64_t>();
			e.xpDrop = entry["xp_drop"].get<int>();
			for (auto& [key, val] : entry["attribs"].items()) {
				e.attribs[key] = val.get<float>();
			}
		}
		MOB_ATTRIBS[tyoe] = std::move(ta);
	}
}

void loadConstants() {
	loadInitSupplies();
	loadTowerAttribs();
	loadMobAttribs();
	loadTalentAttribs();
}
