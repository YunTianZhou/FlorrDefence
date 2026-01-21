#pragma once
#include "Card.hpp"
#include "SharedInfo.hpp"
#include "Constants.hpp"
#include "Petal.hpp"
#include "Mob.hpp"

class Map;
class MapInfo;

class Tower : public sf::Drawable, public sf::Transformable {
public:
	static std::unique_ptr<Tower> create(SharedInfo& info, const CardInfo& card, sf::Vector2i square, Map& map);

public:
	Tower(SharedInfo& info, const CardInfo& card);
	virtual ~Tower();

	virtual void update();
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs);

	void setLength(float length) { m_card.setLength(length); }
	CardInfo getCard() const { return m_card.getCard(); };
	float getAttrib(const std::string& name) const { return m_attribs.attribs.at(name); }
	float getBuffedAttrib(const std::string& name) const { return m_info.playerState.buff.get(name).apply(getAttrib(name)); }

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

protected:
	SharedInfo& m_info;
	const TowerAttribs::RarityEntry& m_attribs;
	TowerCard m_card;
};

class ShootTower : public Tower {
public:
	ShootTower(SharedInfo& info, const CardInfo& card);

	virtual void update() override;
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> getNearestMob(const std::list<std::unique_ptr<Mob>>& mobs) const;

protected:
	sf::Clock m_reloadClock;
};

class DefenceTower : public Tower {
public:
	DefenceTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

	virtual void update() override;
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	sf::Vector2i m_square;
	sf::Clock m_reloadClock;
};

class SummonTower : public Tower {
public:
	SummonTower(SharedInfo& info, const CardInfo& card);

	virtual void update() override;
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	bool ableToSummon();

protected:
	sf::Clock m_reloadClock;
};

class BuffTower : public Tower {
public:
	BuffTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

	virtual void update() override;

	virtual void applyToBuff();

	bool isActive() const;

protected:
	sf::Vector2i m_square;
};

class MultiShotTower : public ShootTower {
public:
	MultiShotTower(SharedInfo& info, const CardInfo& card);

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

private:
	std::vector<std::list<std::unique_ptr<Mob>>::const_iterator>
		getTargets(const std::list<std::unique_ptr<Mob>>& mobs) const;
};

class WebTower : public DefenceTower {
public:
	WebTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

	void update() override;
};

class PollenTower : public DefenceTower {
public:
	using DefenceTower::DefenceTower;

	void update() override;
	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;
};

class ShovelTower : public DefenceTower {
public:
	using DefenceTower::DefenceTower;

	void update() override;
	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

public:
	static const std::unordered_map<std::string, int64_t> coinReward;
};

class RoseTower : public BuffTower {
public:
	RoseTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

	void update() override;

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	sf::Clock m_reloadClock;
};

class CoinTower : public BuffTower {
public:
	CoinTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

	void update() override;

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	sf::Clock m_reloadClock;
};

class TriangleTower : public ShootTower {
public:
	TriangleTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square, const MapInfo& map);

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

private:
	int countAdjacentSameType();

private:
	sf::Vector2i m_square;
	const MapInfo& m_map;
};

class LaserTower : public ShootTower {
public:
	LaserTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square, Map& map);

	void update() override;

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

private:
	sf::Vector2i m_square;
	Map& m_map;
};
