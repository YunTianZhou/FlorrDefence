#pragma once
#include <list>
#include <map>
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "Entity.hpp"
#include "Mob.hpp"

class Petal : public Entity {
public:
	Petal(SharedInfo& info, CardInfo card);
	Petal(SharedInfo& info, CardInfo card, const sf::Texture& texture);  // For summon petal

	virtual void update() {}
	virtual void applyDebuff(Debuff& debuff) const {}

	virtual int getArmor() const override;
	virtual int getDamage() const;
	virtual void onDead() override;

	CardInfo getCard() const { return m_card; }
	int getHp() const { return m_hp; }

protected:
	bool hasAttrib(const std::string& name) const { return m_attribs.attribs.find(name) != m_attribs.attribs.end(); }
	float getAttrib(const std::string& name) const { return m_attribs.attribs.at(name); }
	float getBuffedAttrib(const std::string& name) const { return m_info.playerState.buff.apply(name, getAttrib(name)); }

protected:
	const TowerAttribs::RarityEntry& m_attribs;
	CardInfo m_card;
};

class ShootPetal : public Petal {
public:
	static std::unique_ptr<ShootPetal> create(SharedInfo& info, CardInfo card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target);

public:
	ShootPetal(SharedInfo& info, CardInfo card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target);

	virtual void update() override;
	virtual void updatePosition() override;
	virtual void onDead() override;

	void lostTarget();
	void lostTarget(std::list<std::unique_ptr<Mob>>::const_iterator target);

protected:
	void updateDirection();

protected:
	static const std::map<std::string, sf::Angle> petalTilt;

protected:
	sf::Vector2f m_startPosition;
	std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> m_target;
	sf::Angle m_direction;
};

class DefencePetal : public Petal {
public:
	static std::unique_ptr<DefencePetal> create(SharedInfo& info, CardInfo card, sf::Vector2i square);

public:
	DefencePetal(SharedInfo& info, CardInfo card, sf::Vector2i square);

	virtual void update() override;
	virtual void updatePosition() override;
	virtual void onDead() override;

	sf::Vector2i getSquare() const { return m_square; }

protected:
	sf::Vector2i m_square;
};

class MobPetal : public Petal {
public:
	static std::unique_ptr<MobPetal> create(SharedInfo& info, CardInfo card);

public:
	MobPetal(SharedInfo& info, CardInfo card);

	virtual void update() override;
	virtual void updatePosition() override;

	virtual int getArmor() const override;
	virtual int getDamage() const override;

	const MobAttribs::RarityEntry& getMobAttribs() const { return MOB_ATTRIBS[m_mob.type][m_mob.rarity]; }

private:
	MobInfo m_mob;
	float m_position = 39.f;
};

class WebPetal : public DefencePetal {
public:
	WebPetal(SharedInfo& info, CardInfo card, sf::Vector2i square);

	void update() override;

	int getArmor() const override;
	int getDamage() const override;
	void applyDebuff(Debuff& debuff) const override;

	float getDelta() const;

private:
	sf::Clock m_clock;
};

class TrianglePetal : public ShootPetal {
public:
	TrianglePetal(SharedInfo& info, CardInfo card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target, int adjCount);

	int getDamage() const override;

private:
	int m_adjCount = 0;

};