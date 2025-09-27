#pragma once
#include <list>
#include <SFML/Graphics.hpp>
#include "Mob.hpp"
#include "Tower.hpp"
#include "Petal.hpp"
#include "SharedInfo.hpp"
#include "SpawnManager.hpp"

class MapInfo {
public:
	MapInfo(SharedInfo& info);
	const Tower* getTower(sf::Vector2i square) const;
	Tower* getTower(sf::Vector2i square);
	void setCard(sf::Vector2i square, const CardInfo& card);
	void removeCard(sf::Vector2i square);
	void clear();

public:
	static sf::Vector2i getSquare(sf::Vector2f position);
	static sf::Vector2f getSquareCenter(sf::Vector2i square);

public:
	inline static const sf::Vector2f squareSize = { 100.f, 100.f };

private:
	SharedInfo& m_info;
	std::array<std::array<std::unique_ptr<Tower>, 10>, 11> m_map;
};


class Map : public sf::Drawable {
public:
	Map(SharedInfo& info);

	void update();
	void updateBuff();
	void tick();
	void tickDeadEntities();
	void collision(Petal& petal, Mob& mob);
	void onEvent(const sf::Event& event);

private:
	void handlePress(const sf::Vector2i& square);
	void handleRelease(const sf::Vector2i& square);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	bool isInside(sf::Vector2f position) const;
	void initComponents();
	void initSpawner();

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
	sf::Clock m_tickClock;

	SpawnManager m_spawner;
};
