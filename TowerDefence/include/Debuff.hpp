#pragma once
#include <string>
#include <map>
#include <SFML/System.hpp>
#include "Constants.hpp"

struct RawDedbuff {
    RawDedbuff() = default;

    RawDedbuff(float value_, const std::string rarity, const sf::Time duration_)
        : value(value_), level(RARITIE_LEVELS.at(rarity)), duration(duration_) {
    }

    bool is_active() const {
        return clock.getElapsedTime() < duration;
    }

    void swap(const RawDedbuff& other) {
        if (!is_active() || other.level >= level)
            *this = other;
    }

    virtual float apply(float value) const { return 0.f; };
    float value = 0.f;
    int level = 0;
    sf::Time duration;
    sf::Clock clock;
};

struct SpeedDebuff : public RawDedbuff {
    SpeedDebuff() = default;
    float apply(float speed) const override {
        if (!is_active())
            return speed;
        return speed - speed * value;
    }
};

struct Debuff {
	SpeedDebuff speed;
};
