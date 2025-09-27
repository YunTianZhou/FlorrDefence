#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.hpp"
#include "Debuff.hpp"

class Mob : public Entity {
public:
    static std::unique_ptr<Mob> create(SharedInfo& info, const MobInfo& mob, std::list<std::unique_ptr<Mob>>& mobs);

public:
    Mob(SharedInfo& info, MobInfo mob, float startPosition = 0.f);

    virtual void update();
    virtual void tick();
    virtual void updatePosition() override;

    virtual int getArmor() const override;
    virtual int getDamage() const;
    virtual float getSpeed() const;
    virtual void onDead() override;

    MobInfo getMob() const { return m_mob; }
    Debuff& getDebuff() { return m_debuff; }

protected:
    const MobAttribs::RarityEntry& getAttribs() const { return MOB_ATTRIBS[m_mob.type][m_mob.rarity]; }
    const float getAttrib(const std::string& name) const { return getAttribs().attribs.at(name); }

protected:
    MobInfo m_mob;
    float m_position = 0.f;
    Debuff m_debuff;
};

class SpiderMob : public Mob {
public:
    using Mob::Mob;

    virtual void updatePosition() override;
};

class HornetMob : public Mob {
private:
    enum class State {
        Moving,
        TurningBack,
        WaitingBeforeShoot,
        Shooting,
        WaitingAfterShoot,
        TurningFront
    };

public:
    HornetMob(SharedInfo& info, MobInfo mob, std::list<std::unique_ptr<Mob>>& mobs);

    virtual void update() override;

private:
    void nextShootInterval();

    void shoot();

private:
    std::list<std::unique_ptr<Mob>>& m_mobs;
    sf::Clock m_clock;
    float m_currShootInterval = 0.f;
    State m_state = State::Moving;
};
