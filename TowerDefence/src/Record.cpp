#include "Record.hpp"

#include <iostream>
#include <fstream>
#include "SharedInfo.hpp"
#include "Map.hpp"
#include "Talent.hpp"

Record::Record(SharedInfo& info, MapInfo& map, Talent& talent)
	: m_info(info), m_map(map), m_talent(talent) {

}

bool Record::try_load(std::filesystem::path path) {
	m_info.init();

	std::cout << std::format("Looking for game record from '{}'", path.string()) << std::endl;

	std::ifstream ifs(path);

	if (!ifs.is_open()) {
		std::cout << "Record not found, start a new game." << std::endl;
		return false;
	}

	std::cout << "Loading game record..." << std::endl;

	m_data.clear();
	m_data << ifs;

	m_data["player"].get_to(m_info.playerState);
	m_data["map"].get_to(m_map);
	m_data["talent"].get_to(m_talent);

	std::cout << "Game loaded successfully!" << std::endl;;
	return true;
}

void Record::save(std::filesystem::path path) {
	std::cout << "Saving game..." << std::endl;

	if (m_info.draggedCard.has_value()) {
		std::cout << "[WARNING] Saving game while dragging a card!" << std::endl;
		m_info.playerState.backpack.add({ m_info.draggedCard->getCard(), 1 });
	}

	m_data.clear();
	m_data["player"] = m_info.playerState;
	m_data["map"] = m_map;
	m_data["talent"] = m_talent;

	std::ofstream ofs(path);

	if (!ofs.is_open()) {
		std::cerr << "Failed to save record to " << path << std::endl;
		return;
	}

	ofs << m_data.dump(4);
	ofs.close();
	std::cout << std::format("Game successfully save to '{}'", path.string()) << std::endl;
}
