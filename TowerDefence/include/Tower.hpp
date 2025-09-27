#pragma once
#include "Card.hpp"
#include "SharedInfo.hpp"
#include "Constants.hpp"
#include "Petal.hpp"
#include "Mob.hpp"

class MapInfo;

class Tower : public sf::Drawable, public sf::Transformable {
public:
	static std::unique_ptr<Tower> create(SharedInfo& info, CardInfo card, sf::Vector2i square, const MapInfo& map);

public:
	Tower(SharedInfo& info, CardInfo card);
	virtual ~Tower();

	virtual void update();
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs);

	void setLength(float length) { m_card.setLength(length); }
	CardInfo getCard() const { return m_card.getCard(); };
	float getAttrib(const std::string& name) const { return m_attribs.attribs.at(name); }
	float getBuffedAttrib(const std::string& name) const { return m_info.playerState.buff.apply(name, getAttrib(name)); }

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

protected:
	SharedInfo& m_info;
	const TowerAttribs::RarityEntry& m_attribs;
	TowerCard m_card;
};

class ShootTower : public Tower {
public:
	ShootTower(SharedInfo& info, CardInfo card);

	virtual void update() override;
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> getNearestMob(const std::list<std::unique_ptr<Mob>>& mobs) const;

protected:
	sf::Clock m_reloadClock;
};

class DefenceTower : public Tower {
public:
	DefenceTower(SharedInfo& info, CardInfo card, sf::Vector2i square);

	virtual void update() override;
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	sf::Vector2i m_square;
	sf::Clock m_reloadClock;
};

class SummonTower : public Tower {
public:
	SummonTower(SharedInfo& info, CardInfo card);

	virtual void update() override;
	virtual void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	bool ableToSummon();

protected:
	sf::Clock m_reloadClock;
};

class BuffTower : public Tower {
public:
	BuffTower(SharedInfo& info, CardInfo card, sf::Vector2i square);

	virtual void update() override;

	virtual void applyToBuff();

	bool isActive() const;

protected:
	sf::Vector2i m_square;
};

class MultiShotTower : public ShootTower {
public:
	MultiShotTower(SharedInfo& info, CardInfo card);

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

private:
	std::vector<std::list<std::unique_ptr<Mob>>::const_iterator>
		getTargets(const std::list<std::unique_ptr<Mob>>& mobs) const;
};

class WebTower : public DefenceTower {
public:
	WebTower(SharedInfo& info, CardInfo card, sf::Vector2i square);

	void update() override;
};

class RoseTower : public BuffTower {
public:
	RoseTower(SharedInfo& info, CardInfo card, sf::Vector2i square);

	void update() override;

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

protected:
	sf::Clock m_reloadClock;
};

class TriangleTower : public ShootTower {
public:
	TriangleTower(SharedInfo& info, CardInfo card, sf::Vector2i square, const MapInfo& map);

	void tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) override;

private:
	int countAdjacentSameType();

private:
	sf::Vector2i m_square;
	const MapInfo& m_map;
};
