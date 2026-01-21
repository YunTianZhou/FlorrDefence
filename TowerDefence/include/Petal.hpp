#pragma once
#include <list>
#include <map>
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "Entity.hpp"
#include "Mob.hpp"
#include "Effect.hpp"

class MapInfo;

class Petal : public Entity {
public:
	Petal(SharedInfo& info, const CardInfo& card);
	Petal(SharedInfo& info, const CardInfo& card, const sf::Texture& texture);  // For summon petal

	virtual void update() {}
	virtual void applyDebuff(Debuff& debuff) const {}

	virtual int getArmor() const override;
	virtual int getDamage() const;
	virtual void onDead() override;

	CardInfo getCard() const { return m_card; }
	int getHp() const { return m_hp; }
	DamageType getDamageType() const { return TOWER_ATTRIBS.at(m_card.type).damageType; }

protected:
	bool hasAttrib(const std::string& name) const { return m_attribs.attribs.find(name) != m_attribs.attribs.end(); }
	float getAttrib(const std::string& name) const { return m_attribs.attribs.at(name); }
	float getBuffedAttrib(const std::string& name) const { return m_info.playerState.buff.get(name).apply(getAttrib(name)); }

protected:
	const TowerAttribs::RarityEntry& m_attribs;
	CardInfo m_card;
};

class ShootPetal : public Petal {
public:
	static std::unique_ptr<ShootPetal> create(SharedInfo& info, const CardInfo& card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target);

public:
	ShootPetal(SharedInfo& info, const CardInfo& card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target);
	ShootPetal(SharedInfo& info, const CardInfo& card);  // For laser

	virtual void update() override;
	virtual void updatePosition() override;
	virtual void onDead() override;

	virtual void lostTarget();
	void lostTarget(std::list<std::unique_ptr<Mob>>::const_iterator target);

protected:
	void updateDirection(float trunSpeed);

protected:
	static const std::map<std::string, sf::Angle> petalTilt;

protected:
	sf::Vector2f m_startPosition;
	std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> m_target;
	sf::Angle m_direction;
};

class DefencePetal : public Petal {
public:
	static std::unique_ptr<DefencePetal> create(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

public:
	DefencePetal(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

	virtual void update() override;
	virtual void updatePosition() override;
	virtual void onDead() override;

	sf::Vector2i getSquare() const { return m_square; }

protected:
	sf::Vector2i m_square;
};

class MobPetal : public Petal {
public:
	static std::unique_ptr<MobPetal> create(SharedInfo& info, const CardInfo& card);

public:
	MobPetal(SharedInfo& info, const CardInfo& card, float startPosition = 39.f);

	virtual void update() override;
	virtual void updatePosition() override;

	virtual int getArmor() const override;
	virtual int getDamage() const override;

	const MobAttribs::RarityEntry& getMobAttribs() const { return MOB_ATTRIBS[m_mob.type][m_mob.rarity]; }

private:
	MobInfo m_mob;
	float m_position;
};

class WebPetal : public DefencePetal {
public:
	WebPetal(SharedInfo& info, const CardInfo& card, sf::Vector2i square);

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
	TrianglePetal(SharedInfo& info, const CardInfo& card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target, int adjCount);

	int getDamage() const override;

private:
	int m_adjCount = 0;

};

class LightningPetal : public ShootPetal {
public:
	using ShootPetal::ShootPetal;

	void onHit(Mob& mob, std::list<std::unique_ptr<Mob>>& mobs, std::list<std::unique_ptr<Effect>>& effects);

private:
	std::vector<std::list<std::unique_ptr<Mob>>::iterator> getTargets(std::list<std::unique_ptr<Mob>>& mobs) const;
};

class PincerPetal : public ShootPetal {
public:
	using ShootPetal::ShootPetal;

	void applyDebuff(Debuff& debuff) const override;
};

class JellyPetal : public DefencePetal {
public:
	using DefencePetal::DefencePetal;

	void applyDebuff(Debuff& debuff) const override;
};

class DicePetal : public ShootPetal {
public:
	using ShootPetal::ShootPetal;

	int getDamage() const override;

private:
	inline static const float boostProb = 0.05f;
	inline static const int boostRate = 35;
};

class BurPetal : public ShootPetal {
public:
	using ShootPetal::ShootPetal;

	void applyDebuff(Debuff& debuff) const override;
};

class LaserPetal : public ShootPetal {
public:
	LaserPetal(SharedInfo& info, const CardInfo& card, sf::Vector2i square, MapInfo& map, const std::list<std::unique_ptr<Mob>>& mobs);

	int getArmor() const override;
	int getDamage() const override;

	void onDead() override;
	void update() override;
	void updatePosition() override;

private:
	void updateTarget();

public:
	sf::Time getDeathDuration() const override { return sf::Time::Zero; }

private:
	const std::list<std::unique_ptr<Mob>>& m_mobs;
	MapInfo& m_map;
	sf::Vector2i m_square;
	sf::Clock m_clock;
};

class ChipPetal : public ShootPetal {
public:
	using ShootPetal::ShootPetal;

	int getArmor() const override;
};
