#include "Mob.hpp"
#include "AssetManager.hpp"
#include "Tools.hpp"
#include "Map.hpp"

// Mob
std::unique_ptr<Mob> Mob::create(SharedInfo& info, const MobInfo& mob, std::list<std::unique_ptr<Mob>>& mobs) {
    if (mob.type == "spider")
        return std::make_unique<SpiderMob>(info, mob);
    if (mob.type == "hornet")
        return std::make_unique<HornetMob>(info, mob, mobs);
    return std::make_unique<Mob>(info, mob);
}

Mob::Mob(SharedInfo& info, MobInfo mob, float startPosition)
    : m_mob(mob), m_position(startPosition), Entity(info, AssetManager::getMobTexture(mob.type)) {
    setScale(MOB_RARITY_SCALES.at(mob.rarity));
    setFlash(sf::Color(255, 200, 200), 0.9f);
    m_hp = getAttribs().hp;
}

void Mob::update() {
    // Animation
    updateAnimation();

    // Position
    updatePosition();
}

void Mob::tick() {
    auto& player = m_info.playerState;

    // Hit player logic
    if (m_position >= 39.f) {
        player.hit(getAttribs().damage);
        hit(player.getBodyDamage());
    }
}

void Mob::updatePosition() {
    // Movement along path
    if (m_position < 39.f) {
        m_position = std::min(39.f, m_position + m_info.dt.asSeconds() * getSpeed());
        updatePathPosition(m_position);
    }
}

int Mob::getArmor() const {
    return getAttribs().armor;
}

int Mob::getDamage() const {
    return getAttribs().damage;
}

float Mob::getSpeed() const {
    return m_debuff.speed.apply(getAttribs().speed);
}

void Mob::onDead() {
    auto& player = m_info.playerState;

    player.addCoin(getAttribs().coinDrop);
    player.addXp(getAttribs().xpDrop);
}

// Spider Mob
void SpiderMob::updatePosition() {
    // Movement along path
    if (m_position < 39.f) {
        m_position = std::min(39.f, m_position + m_info.dt.asSeconds() * getSpeed());
        rotate(sf::degrees(getAttrib("rotation_speed") * m_info.dt.asSeconds()));
        updatePathPosition(m_position);
    }
}

// Hornet Mob
HornetMob::HornetMob(SharedInfo& info, MobInfo mob, std::list<std::unique_ptr<Mob>>& mobs) 
    : m_mobs(mobs), Mob(info, mob) {
    nextShootInterval();
}

void HornetMob::update() {
    // Animation
    updateAnimation();

    switch (m_state) {
    case State::Moving: {
        updatePosition();

        if (m_position < 36.f &&
            m_clock.getElapsedTime().asSeconds() >= m_currShootInterval)
        {
            m_state = State::TurningBack;
        }
        break;
    }

    case State::TurningBack: {
        float curr = getRotationOffset().asDegrees();

        if (curr < 180.f) {
            float offset = getAttrib("rotation_speed") * m_info.dt.asSeconds();
            sf::Angle next = sf::degrees(std::min(180.f, curr + offset));
            setRotationOffset(next);
            updatePathPosition(m_position);
        }
        else {
            m_state = State::WaitingBeforeShoot;
            m_clock.restart();
        }
        break;
    }

    case State::WaitingBeforeShoot: {
        if (m_clock.getElapsedTime().asSeconds() >= getAttrib("pre_shoot_delay")) {
            m_state = State::Shooting;
        }
        break;
    }

    case State::Shooting: {
        shoot();
        m_clock.restart();
        m_state = State::WaitingAfterShoot;
        break;
    }

    case State::WaitingAfterShoot: {
        if (m_clock.getElapsedTime().asSeconds() >= getAttrib("post_shoot_delay")) {
            m_state = State::TurningFront;
        }
        break;
    }

    case State::TurningFront: {
        float curr = getRotationOffset().asDegrees();

        if (curr > 0.f) {
            float offset = getAttrib("rotation_speed") * m_info.dt.asSeconds();
            sf::Angle next = sf::degrees(std::max(0.f, curr - offset));
            setRotationOffset(next);
            updatePathPosition(m_position);
        }
        else {
            m_state = State::Moving;
            m_clock.restart();

            nextShootInterval();
        }
        break;
    }
    }
}

void HornetMob::nextShootInterval() {
    float base = getAttrib("shoot_interval");
    float jitter = getAttrib("shoot_interval_jitter");
    m_currShootInterval = base + randomUniform(-jitter, jitter);
}

void HornetMob::shoot() {
    m_mobs.push_back(std::make_unique<Mob>(m_info, MobInfo{ m_mob.rarity, "missile" }, m_position));
}
