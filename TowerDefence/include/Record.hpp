#pragma once
#include <filesystem>
#include <nlohmann/json.hpp>

using nlohmann::json;

struct SharedInfo;
class Map;
class Shop;
class Talent;

class Record {
public:
	Record(SharedInfo& info, Map& map, Shop& shop, Talent& talent);

	bool try_load(std::filesystem::path path);

	void save(std::filesystem::path path);

private:
	SharedInfo& m_info;
	Map& m_map;
	Shop& m_shop;
	Talent& m_talent;
	json m_data;
};
