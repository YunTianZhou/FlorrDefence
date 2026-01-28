#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.hpp"
#include "Debuff.hpp"

class Mob : public Entity {
public:
    static std::unique_ptr<Mob> create(SharedInfo& info, const MobInfo& mob, std::list<std::unique_ptr<Mob>>& mobs);

public:
    Mob(SharedInfo& info, const MobInfo& mob, float startPosition = 0.f);

    virtual void update();
    virtual void tick();
    virtual void updatePosition() override;

    virtual int getArmor() const override;
    virtual int getDamage() const;
    virtual float getSpeed() const;
    virtual float getSlowDownResistance() const;
    virtual float getKnockbackResistance() const;

    virtual void onDead() override;

    MobInfo getMob() const { return m_mob; }
    Debuff& getDebuff() { return m_debuff; }
    float getPathPosition() const { return m_position; }

    friend void to_json(json& j, const Mob& m);
    friend void from_json(const json& j, Mob& m);

protected:
    inline static const float knockbackThreshold = 0.05f;
    inline static const float knockbackDecayFactor = 0.08f;
    inline static const float knockbackBlendRange = 0.8f;
        
    static const std::unordered_map<std::string, float> raritySlowDownResistance;
    static const std::unordered_map<std::string, float> rarityKnockbackResistance;

protected:
    const MobAttribs::RarityEntry& getAttribs() const { return MOB_ATTRIBS[m_mob.type][m_mob.rarity]; }
    const float getAttrib(const std::string& name) const { return getAttribs().attribs.at(name); }

protected:
    MobInfo m_mob;
    float m_position = 0.f;
    float m_knockback = 0.f;
    Debuff m_debuff;
};

inline void to_json(json& j, const Mob& m) {
    j["card"] = m.getMob();
    j["hp"] = m.m_hp;
    j["position"] = m.m_position;
}

inline void from_json(const json& j, Mob& m) {
    assert(m.getMob() == j.value("card", MobInfo{}));
    m.m_hp = j.value("hp", 0);
    m.m_position = j.value("position", 0.f);
}

class SpiderMob : public Mob {
public:
    using Mob::Mob;

    void updatePosition() override;
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
    HornetMob(SharedInfo& info, const MobInfo& mob, std::list<std::unique_ptr<Mob>>& mobs);

    void update() override;

    float getSpeed() const;

private:
    void nextShootInterval();

    void shoot();

private:
    std::list<std::unique_ptr<Mob>>& m_mobs;
    sf::Time m_timer;
    float m_currShootInterval = 0.f;
    State m_state = State::Moving;
};

class RoachMob : public Mob {
private:
    enum class State {
        Moving,
        SpeedUp,
        Running,
        SlowDown
    };

public:
    RoachMob(SharedInfo& info, const MobInfo& mob);

    void update() override;

    float getSpeed() const override;

private:
    void nextPeriod();

private:
    float m_speed;
    sf::Time m_timer;
    float m_currRestTime = 0.f;
    float m_currRunningTime = 0.f;
    State m_state = State::Moving;
};

class FlyMob : public Mob {
public:
    using Mob::Mob;

    void updatePosition() override;

    void hit(int damage, DamageType type) override;

private:
    float m_headDeg = 0.f;
};
