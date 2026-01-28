#pragma once

#include <random>
#include "SharedInfo.hpp"

class Mob;

struct MobTypeEntry {
    MobInfo mob;
    double weight = 1.0;
};

struct JitterConfig { 
    double range = 0.0; 
    double prob = 1.0; 
};

struct OscConfig { 
    bool enabled = false; 
    double period = 30.0; 
    double amplitude = 0.0; 
};

struct Stage {
    int min_level = 0;
    int max_level = 0;
    double base_interval = 2.5;
    double scale_per_level = 0.0;
    JitterConfig jitter;
    OscConfig oscillator;
    std::vector<MobTypeEntry> mob_types;
};

class SpawnManager {
public:
    explicit SpawnManager(SharedInfo& info);

    void load();

    void update(std::list<std::unique_ptr<Mob>>& mobList);

private:
    Stage const* findStage(int level) const;
    const MobTypeEntry* chooseMobType(const Stage& s);

private:
    double computeNextInterval(const Stage& s, int level);

private:
    SharedInfo& m_info;
    std::vector<Stage> m_stages;
    size_t m_maxMob = 200;

    // timing
    sf::Time m_spawnTimer;
    sf::Time m_globalTimer;
    double m_nextInterval = 2.5;
    double m_prevInterval = 2.5;

    // rng
    std::mt19937 m_rng;

    // global clamp
    double m_globalMinInterval = 0.5;
    double m_globalMaxInterval = 10.0;
    double m_globalSmoothingAlpha = 0.2;
};
