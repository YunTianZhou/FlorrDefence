#pragma once

#include <list>
#include <random>
#include "SharedInfo.hpp"
#include "EncounterPlan.hpp"

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
    explicit SpawnManager(SharedInfo* info);

    void load();

    void update(std::list<std::unique_ptr<Mob>>& mobList);
    void startEarly();
    nlohmann::json save() const;
    void restore(const nlohmann::json& state);

private:
    void loadLegacy();
    void updateEncounter(std::list<std::unique_ptr<Mob>>& mobs);
    void prepareEncounter();
    void updateStatus(size_t alive);
    bool m_legacy = false;
    nlohmann::json m_config;
    std::unique_ptr<EncounterPlan> m_rules;
    std::vector<MobInfo> m_queue;
    std::vector<double> m_strengths;
    size_t m_spawned = 0;
    int m_wave = 1;
    unsigned m_seed = 0;
    bool m_preparing = true;
    bool m_bossRound = false;
    double m_remaining = 0, m_interval = 1;
    std::string m_theme;

    Stage const* findStage(int level) const;
    const MobTypeEntry* chooseMobType(const Stage& s);

private:
    double computeNextInterval(const Stage& s, int level);

private:
    SharedInfo* m_info;
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
    double m_globalMinInterval = 0.2;
    double m_globalMaxInterval = 10.0;
    double m_globalSmoothingAlpha = 0.2;
};
