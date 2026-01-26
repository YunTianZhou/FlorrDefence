#pragma once
#include <list>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include "Mob.hpp"
#include "Tower.hpp"
#include "Petal.hpp"
#include "SharedInfo.hpp"
#include "SpawnManager.hpp"
#include "Effect.hpp"

class Map;

class MapInfo {
public:
	MapInfo(SharedInfo& info, Map& map);
	const Tower* getTower(sf::Vector2i square) const;
	Tower* getTower(sf::Vector2i square);
	void setCard(sf::Vector2i square, const CardInfo& card);
	void removeCard(sf::Vector2i square);
	int removeAll(const CardInfo& card);
	void clear();
	void updateTowerBuff();

	void update();
	void tick();

	bool isValid(sf::Vector2i square) const;
	bool isEmpty(sf::Vector2i square) const;
	bool isPlaceable(sf::Vector2i square, const CardInfo& card) const;
	bool findSquareAndPlace(const CardInfo& card);

public:
	static sf::Vector2i getSquare(sf::Vector2f position);
	static sf::Vector2f getSquareCenter(sf::Vector2i square);

public:
	enum class SquareType {
		Grass,
		Trail,
		Obstacle,
		Slot
	};

	inline static const sf::Vector2f squareSize = { 100.f, 100.f };
	static std::array<std::array<SquareType, 10>, 11> m_squareTypeMap;

private:
	Map& m_mapRef;

	SharedInfo& m_info;
	std::array<std::array<std::unique_ptr<Tower>, 10>, 11> m_map;

	bool m_buffUpdated = false;
};

inline void to_json(json& j, const MapInfo& m) {
    j["towers"] = json::array();

    for (int x = 0; x < 11; x++) {
        for (int y = 0; y < 10; y++) {
            sf::Vector2i square{x, y};
            const Tower* tower = m.getTower(square);
            if (!tower) continue;

            j["towers"].push_back({
                { "x", x },
                { "y", y },
                { "card", tower->getCard() }
            });
        }
    }
}

inline void from_json(const json& j, MapInfo& m) {
	m.clear();

	if (!j.contains("towers"))
		return;

	for (const auto& e : j["towers"]) {
		sf::Vector2i square{
			e.at("x").get<int>(),
			e.at("y").get<int>()
		};

		CardInfo card = e.at("card").get<CardInfo>();
		m.setCard(square, card);
	}
}

class Map : public sf::Drawable {
public:
	Map(SharedInfo& info);

	bool update();
	void tick();
	void tickDeadEntities();
	void collision(Petal& petal, Mob& mob);
	bool onEvent(const sf::Event& event);

	const MapInfo& getMapInfo() const { return m_map; }
	MapInfo& getMapInfo() { return m_map; }

	const std::list<std::unique_ptr<Mob>>& getMobs() const { return m_mobs; }
	std::list<std::unique_ptr<Mob>>& getMobs() { return m_mobs; }

	const std::list<std::unique_ptr<Petal>>& getPetals() const { return m_petals; }
	std::list<std::unique_ptr<Petal>>& getPetals() { return m_petals; }

private:
	void handlePress(const sf::Vector2i& square);
	bool handleRightPress(const sf::Vector2i& square);
	void handleRelease(const sf::Vector2i& square);
	bool handlePlaceTowerRequest();
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	bool isInside(sf::Vector2f position) const;
	void initComponents();

public:
	inline static const sf::FloatRect bounds = sf::FloatRect({ 0.f, 0.f }, { 1000.f, 1100.f });

private:
	SharedInfo& m_info;
	MapInfo m_map;
	sf::RectangleShape m_background;
	mutable sf::RectangleShape m_highlight;

	std::list<std::unique_ptr<Mob>> m_mobs;
	std::list<std::unique_ptr<Petal>> m_petals;
	std::list<std::unique_ptr<Entity>> m_deadEntities;
	std::list<std::unique_ptr<Effect>> m_effects;
	sf::Clock m_tickClock;

	SpawnManager m_spawner;
};
