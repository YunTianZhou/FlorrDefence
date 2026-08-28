#include "Record.hpp"

#include <iostream>
#include <fstream>
#include <format>
#include <chrono>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "Game.hpp"

namespace {
	void replaceFile(const std::filesystem::path& source, const std::filesystem::path& destination) {
#ifdef _WIN32
		if (!MoveFileExW(source.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
			throw std::filesystem::filesystem_error(
				"Failed to replace save file",
				source,
				destination,
				std::error_code(GetLastError(), std::system_category())
			);
		}
#else
		std::filesystem::rename(source, destination);
#endif
	}
}

Record& Record::instance() {
	static Record record;
	return record;
}

bool Record::try_load(Game& game, const std::filesystem::path& path) {
	if (path.empty()) {
		std::cout << "Game record path is empty, start a new game by default." << std::endl;
		return true;
	}

	std::cout << std::format("Looking for game record from '{}'", path.string()) << std::endl;

	std::ifstream ifs(path);

	if (!ifs.is_open()) {
		std::cout << "Record not found, start a new game." << std::endl;
		return true;
	}

	std::cout << "Loading game record..." << std::endl;

	try {
		m_data.clear();
		ifs >> m_data;

		m_data["player"].get_to(game.m_info.playerState);
		m_data["map"].get_to(game.m_map);
		m_data["shop"].get_to(game.m_ui.m_shop);
		m_data["talent"].get_to(game.m_ui.m_talent);

		auto& uniques = game.m_info.playerState.aquiredUniques;

		for (const std::string& type : TOWER_TYPES) {
			CardInfo card = { "unique", type };

			if (game.m_info.playerState.backpack.getCount(card) > 0
				|| game.m_map.getMapInfo().containsTower(card)) {
				uniques.insert(type);
			}
		}

		std::cout << "Game loaded successfully!" << std::endl;
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load game record: " << e.what() << std::endl;
		return false;
	}
}

bool Record::save(const Game& game, const std::filesystem::path& path) {
	if (path.empty()) {
		std::cout << "Target saving file is empty, do not save by default." << std::endl;
		return false;
	}

	std::cout << "Saving game..." << std::endl;

	std::filesystem::path temporaryPath;
	try {
		json data;
		data["player"] = game.m_info.playerState;

		if (game.m_info.draggedCard.has_value()) {
			BackpackInfo backpack = game.m_info.playerState.backpack;
			backpack.add({ game.m_info.draggedCard->getCard(), 1 });
			data["player"]["backpack"] = backpack;
		}

		data["map"] = game.m_map;
		data["shop"] = game.m_ui.m_shop;
		data["talent"] = game.m_ui.m_talent;

		temporaryPath = path;
		auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
		temporaryPath += ".tmp." + std::to_string(timestamp);

		{
			std::ofstream ofs(temporaryPath, std::ios::binary | std::ios::trunc);
			ofs.exceptions(std::ios::failbit | std::ios::badbit);
			ofs << data.dump(4);
			ofs.flush();
			ofs.close();
		}

		replaceFile(temporaryPath, path);
		m_data = std::move(data);

		std::cout << std::format(
			"Game successfully saved to '{}'",
			path.string()
		) << std::endl;
		return true;
	}
	catch (const std::exception& e) {
		if (!temporaryPath.empty()) {
			std::error_code error;
			std::filesystem::remove(temporaryPath, error);
		}
		std::cerr << "Failed to save record: " << e.what() << std::endl;
		return false;
	}
}
