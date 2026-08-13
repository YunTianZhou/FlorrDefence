#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>

class Game;

using nlohmann::json;

class Record {
public:
	static Record& instance();

	Record(const Record&) = delete;
	Record& operator=(const Record&) = delete;
	Record(Record&&) = delete;
	Record& operator=(Record&&) = delete;

	bool try_load(Game& game, const std::filesystem::path& path);
	void save(Game& game, const std::filesystem::path& path);

private:
	Record() = default;
	~Record() = default;

	json m_data;
};
