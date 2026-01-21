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
    if (mob.type == "roach")
        return std::make_unique<RoachMob>(info, mob);
    if (mob.type == "fly")
        return std::make_unique<FlyMob>(info, mob);
    return std::make_unique<Mob>(info, mob);
}

const std::unordered_map<std::string, float> Mob::raritySlowDownResistance = {
    {"common", 1.f },
    {"unusual", 0.95f },
    {"rare", 0.9f },
    {"epic", 0.8f },
    {"legendary", 0.7f },
    {"mythic", 0.6f },
    {"ultra", 0.5f },
    {"super", 0.3f }
};

const std::unordered_map<std::string, float> Mob::rarityKnockbackResistance = {
    {"common", 1.f },
    {"unusual", 0.9f },
    {"rare", 0.8f },
    {"epic", 0.7f },
    {"legendary", 0.6f },
    {"mythic", 0.5f },
    {"ultra", 0.3f },
    {"super", 0.1f }
};

Mob::Mob(SharedInfo& info, const MobInfo& mob, float startPosition)
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
    float resistance = getSlowDownResistance();
    float baseSpeed = m_debuff.webSpeed.apply(getSpeed(), resistance);
    baseSpeed = m_debuff.pincerSpeed.apply(baseSpeed, resistance);
    float speed = baseSpeed;

    if (m_knockback >= knockbackThreshold) {
        float t = std::clamp(m_knockback / knockbackBlendRange, 0.f, 1.f);
        t = t * t * (3.f - 2.f * t);

        float forwardPart = baseSpeed * (1.f - t);
        speed = forwardPart - m_knockback;
    }

    m_position = std::clamp(m_position + m_info.dt.asSeconds() * speed, 0.f, 39.f);

    m_knockback = std::max(m_knockback, m_debuff.knockback.get(getKnockbackResistance()));
    if (m_knockback >= knockbackThreshold) {
        float factor = std::pow(knockbackDecayFactor, m_info.dt.asSeconds());
        m_knockback *= factor;

        if (m_knockback < knockbackThreshold)
            m_knockback = 0.f;
    }

    updatePathPosition(m_position);
}

int Mob::getArmor() const {
    return (int)m_debuff.armor.apply((float)getAttribs().armor);
}

int Mob::getDamage() const {
    return getAttribs().damage;
}

float Mob::getSpeed() const {
    return getAttribs().speed;
}

float Mob::getSlowDownResistance() const {
    return raritySlowDownResistance.at(m_mob.rarity);
}

float Mob::getKnockbackResistance() const {
    return rarityKnockbackResistance.at(m_mob.rarity);
}

void Mob::onDead() {
    auto& player = m_info.playerState;

    player.addCoin(getAttribs().coinDrop);
    player.addXp(getAttribs().xpDrop);
}

// Spider Mob
void SpiderMob::updatePosition() {
    rotate(sf::degrees(getAttrib("rotation_speed") * m_info.dt.asSeconds()));

    // Movement along path
    Mob::updatePosition();
}

// Hornet Mob
HornetMob::HornetMob(SharedInfo& info, const MobInfo& mob, std::list<std::unique_ptr<Mob>>& mobs)
    : m_mobs(mobs), Mob(info, mob) {
    nextShootInterval();
}

void HornetMob::update() {
    switch (m_state) {
    case State::Moving: {
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
        }
        else {
            m_state = State::Moving;
            m_clock.restart();

            nextShootInterval();
        }
        break;
    }
    }

    // Animation
    updateAnimation();

    // Position
    updatePosition();
}

float HornetMob::getSpeed() const {
    if (m_state == State::Moving)
        return getAttribs().speed;
    else 
        return 0.f;
};

void HornetMob::nextShootInterval() {
    float base = getAttrib("shoot_interval");
    float jitter = getAttrib("shoot_interval_jitter");
    m_currShootInterval = base + randomUniform(-jitter, jitter);
}

void HornetMob::shoot() {
    m_mobs.push_back(std::make_unique<Mob>(m_info, MobInfo{ m_mob.rarity, "missile" }, m_position));
}

// Roach
RoachMob::RoachMob(SharedInfo& info, const MobInfo& mob)
    : Mob(info, mob), m_speed(getAttribs().speed) {
    nextPeriod();
}

static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void RoachMob::update() {
    const float baseSpeed = getAttribs().speed;
    const float runningSpeed = getAttrib("running_speed");
    const float speedUpDur = getAttrib("speed_up_duration");
    const float slowDownDur = getAttrib("slow_down_duration");
    const float elapsed = m_clock.getElapsedTime().asSeconds();

    switch (m_state) {
    case State::Moving:
        m_speed = baseSpeed;
        if (elapsed >= m_currRestTime) {
            m_state = State::SpeedUp;
            m_clock.restart();
        }
        break;

    case State::SpeedUp: {
        float t = elapsed / speedUpDur;
        if (t >= 1.f) {
            m_speed = runningSpeed;
            m_state = State::Running;
            m_clock.restart();
        }
        else {
            m_speed = lerp(baseSpeed, runningSpeed, t);
        }
        break;
    }

    case State::Running:
        m_speed = runningSpeed;
        if (elapsed >= m_currRunningTime) {
            m_state = State::SlowDown;
            m_clock.restart();
        }
        break;

    case State::SlowDown: {
        float t = elapsed / slowDownDur;
        if (t >= 1.f) {
            m_speed = baseSpeed;
            m_state = State::Moving;
            m_clock.restart();

            nextPeriod();
        }
        else {
            m_speed = lerp(runningSpeed, baseSpeed, t);
        }
    }
    }

    // Animation
    updateAnimation();

    // Position
    updatePosition();
}

float RoachMob::getSpeed() const {
    return m_speed;
}

void RoachMob::nextPeriod() {
    float restBase = getAttrib("rest_duration");
    float restJitter = getAttrib("rest_duration_jitter");
    m_currRestTime = restBase + randomUniform(-restJitter, restJitter);

    float runningBase = getAttrib("running_duration");
    float runningJitter = getAttrib("running_duration_jitter");
    m_currRunningTime = runningBase + randomUniform(-runningJitter, runningJitter);
}

// Fly
void FlyMob::updatePosition() {
    const float dt = m_info.dt.asSeconds();

    float rotationSpeed = getAttrib("rotation_speed");
    float range = getAttrib("rotation_range");
    const float cycle = 4.f * range;
    m_headDeg = fmod(m_headDeg + rotationSpeed * dt, cycle);

    float offsetDeg = m_headDeg < 2.f * range ? -range + m_headDeg : (3.f * range) - m_headDeg;
    setRotationOffset(sf::degrees(offsetDeg));

    Mob::updatePosition();
}

void FlyMob::hit(int damage, DamageType type) {
    if (type == DamageType::Normal && randomUniform(0.f, 1.f) <= getAttrib("evasion"))
        return;
    Mob::hit(damage, type);
}
