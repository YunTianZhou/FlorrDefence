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
	Record(SharedInfo* info, Map& map, Shop& shop, Talent& talent);
	bool try_load();
	bool try_load(std::filesystem::path path);

	void save();
	void save(std::filesystem::path path);

public:
	static std::filesystem::path defaultLoadPath;
	static std::filesystem::path defaultSavePath;

private:
	SharedInfo* m_info;
	Map& m_map;
	Shop& m_shop;
	Talent& m_talent;
	json m_data;
};
