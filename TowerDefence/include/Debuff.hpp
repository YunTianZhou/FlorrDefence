#pragma once
#include <string>
#include <map>
#include <SFML/System.hpp>
#include "Constants.hpp"

struct DurationDebuff {
    DurationDebuff() = default;

    DurationDebuff(float value_, const std::string rarity, const sf::Time duration_)
        : value(value_), level(RARITIE_LEVELS.at(rarity)), duration(duration_) {
    }

    void update(sf::Time dt) {
        timer += dt;
    }

    bool is_active() const {
        return timer < duration;
    }

    void swap(const DurationDebuff& other) {
        if (!is_active() || other.level >= level)
            *this = other;
    }

    virtual float apply(float value, float resistance) const { return 0.f; };

    float value = 0.f;
    int level = 0;
    sf::Time duration;
    sf::Time timer;
};

struct ValueDebuff {
    ValueDebuff() = default;

    ValueDebuff(float value_, const std::string rarity)
        : value(value_), level(RARITIE_LEVELS.at(rarity)), active(true) {}

    bool is_active() const {
        return active;
    }

    void swap(const ValueDebuff& other) {
        if (!is_active() || other.level >= level)
            *this = other;
    }

    float get(float resistance) const {
        if (!is_active())
            return 0.f;
        active = false;
        return resistance * value;
    }

    float value = 0.f;
    int level = 0;
    mutable bool active = false;
};

struct SpeedDebuff : public DurationDebuff {
    SpeedDebuff() = default;

    float apply(float speed, float resistance) const override {
        if (!is_active())
            return speed;
        return speed * (1 - value * resistance);
    }
};

struct ArmorDebuff : public DurationDebuff {
    ArmorDebuff() = default;

    float apply(float armor, float resistance = 1.f) const override {
        if (!is_active())
            return armor;
        return std::max(0.f, armor - value * resistance);
    }
};

struct Debuff {
    SpeedDebuff webSpeed;
    SpeedDebuff pincerSpeed;
    ValueDebuff knockback;
    ArmorDebuff armor;

    void update(sf::Time dt) {
        webSpeed.update(dt);
        pincerSpeed.update(dt);
        armor.update(dt);
    }
};
