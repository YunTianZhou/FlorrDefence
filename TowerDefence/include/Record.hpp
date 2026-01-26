#pragma once
#include <filesystem>
#include <nlohmann/json.hpp>

using nlohmann::json;

struct SharedInfo;
class MapInfo;
class Talent;

class Record {
public:
	Record(SharedInfo& info, MapInfo& map, Talent& talent);

	bool try_load(std::filesystem::path path);

	void save(std::filesystem::path path);

private:
	SharedInfo& m_info;
	MapInfo& m_map;
	Talent& m_talent;
	json m_data;
};
