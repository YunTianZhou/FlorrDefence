#include "Record.hpp"

#include <iostream>
#include <fstream>
#include <format>

#include "Game.hpp"

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
		return false;
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

void Record::save(Game& game, const std::filesystem::path& path) {
	if (path.empty()) {
		std::cout << "Target saving file is empty, do not save by default." << std::endl;
		return;
	}

	std::cout << "Saving game..." << std::endl;

	if (game.m_info.draggedCard.has_value()) {
		std::cout << "[WARNING] Saving game while dragging a card!" << std::endl;
		game.m_info.playerState.backpack.add({ game.m_info.draggedCard->getCard(), 1 });
	}

	try {
		m_data.clear();
		m_data["player"] = game.m_info.playerState;
		m_data["map"] = game.m_map;
		m_data["shop"] = game.m_ui.m_shop;
		m_data["talent"] = game.m_ui.m_talent;

		std::ofstream ofs(path);

		if (!ofs.is_open()) {
			std::cerr << "Failed to save record to " << path << std::endl;
			return;
		}

		ofs << m_data.dump(4);

		std::cout << std::format(
			"Game successfully saved to '{}'",
			path.string()
		) << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to save record: " << e.what() << std::endl;
	}
}
