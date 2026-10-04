#pragma once

#include <list>
#include <random>
#include "SharedInfo.hpp"
#include "EncounterPlan.hpp"

class Mob;

class SpawnManager {
public:
    explicit SpawnManager(SharedInfo* info);

    void load();

    void update(std::list<std::unique_ptr<Mob>>& mobList);
    void startEarly();
    void advanceWaves(int amount);
    nlohmann::json save() const;
    void restore(const nlohmann::json& state);

private:
    void prepareEncounter();
    void updateStatus(size_t alive);
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

    SharedInfo* m_info;
    size_t m_maxMob = 100;
};
