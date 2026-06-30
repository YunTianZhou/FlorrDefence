#include "SpawnManager.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Mob.hpp"

SpawnManager::SpawnManager(SharedInfo& info)
    : m_info(info)
{
    std::random_device rd;
    m_rng.seed(rd());
    load();
}

void SpawnManager::load() {
    std::ifstream ifs("res/config/mob_spawn_config.json");
    if (!ifs.is_open())
        throw std::runtime_error("Failed to open mob_spawn_config.json");

    nlohmann::json j;
    ifs >> j;

    auto gj = j["global"];
    if (gj.contains("min_interval")) m_globalMinInterval = gj["min_interval"].get<double>();
    if (gj.contains("max_interval")) m_globalMaxInterval = gj["max_interval"].get<double>();
    if (gj.contains("smoothing_alpha")) m_globalSmoothingAlpha = gj["smoothing_alpha"].get<double>();

    m_maxMob = j["max_mob"].get<int>();

    m_stages.clear();
    for (const auto& sj : j["stages"]) {
        Stage st;
        st.min_level = sj["min_level"].get<int>();
        st.max_level = sj["max_level"].get<int>();
        st.base_interval = sj.value("base_interval", st.base_interval);
        st.scale_per_level = sj.value("scale_per_level", 0.0);

        // jitter
        if (sj.contains("jitter")) {
            st.jitter.range = sj["jitter"].value("range", st.jitter.range);
            st.jitter.prob = sj["jitter"].value("prob", st.jitter.prob);
        }

        // oscillator
        if (sj.contains("oscillator")) {
            st.oscillator.enabled = true;
            st.oscillator.period = sj["oscillator"].value("period", st.oscillator.period);
            st.oscillator.amplitude = sj["oscillator"].value("amplitude", st.oscillator.amplitude);
        }

        for (const auto& m : sj["mob_types"]) {
            MobTypeEntry e;
            e.mob.type = m["type"].get<std::string>();
            e.mob.rarity = m.value("rarity", std::string("common"));
            e.weight = m.value("weight", 1.0);
            st.mob_types.push_back(std::move(e));
        }

        m_stages.push_back(std::move(st));
    }

    // init timing vars
    if (!m_stages.empty()) {
        m_nextInterval = m_stages.front().base_interval;
        m_prevInterval = m_nextInterval;
    }
}

Stage const* SpawnManager::findStage(int level) const {
    for (const auto& s : m_stages) {
        if (level >= s.min_level && level <= s.max_level)
            return &s;
    }
    return nullptr;
}

const MobTypeEntry* SpawnManager::chooseMobType(const Stage& s) {
    double total = 0.0;
    for (const auto& e : s.mob_types) 
        total += e.weight;
    if (s.mob_types.empty() || total <= 0.0) return nullptr;

    std::uniform_real_distribution<double> dist(0.0, total);
    double r = dist(m_rng);
    double accum = 0.0;
    for (const auto& e : s.mob_types) {
        accum += e.weight;
        if (r <= accum) return &e;
    }

    return &s.mob_types.front();
}

void SpawnManager::update(std::list<std::unique_ptr<Mob>>& mobList) {
    m_spawnTimer += m_info.dt;
    m_globalTimer += m_info.dt;

    if (mobList.size() >= m_maxMob) return;
    if (m_spawnTimer.asSeconds() < m_nextInterval) return;
    m_spawnTimer = sf::Time::Zero;

    int playerLevel = m_info.playerState.level;
    const Stage* stage = findStage(playerLevel);
    if (!stage) return;

    const MobTypeEntry* pick = chooseMobType(*stage);
    if (!pick) return;

    auto mobPtr = Mob::create(m_info, pick->mob, mobList);
    if (mobPtr) {
        mobList.push_back(std::move(mobPtr));
    }

    m_nextInterval = computeNextInterval(*stage, playerLevel);
}

double SpawnManager::computeNextInterval(const Stage& s, int level) {
    double t = m_globalTimer.asSeconds();
    double raw = s.base_interval;

    // Apply linear decrease per level (scale_per_level usually negative)
    int levelOffset = level - s.min_level;
    raw += s.scale_per_level * (double)(levelOffset);

    // oscillator (sine)
    if (s.oscillator.enabled && s.oscillator.period > 0.0) {
        double phase = (2.0 * 3.14159265358979323846 * t) / s.oscillator.period;
        raw += std::sin(phase) * s.oscillator.amplitude;
    }

    // jitter (instant)
    if (s.jitter.range > 0.0) {
        std::uniform_real_distribution<double> ud(-s.jitter.range, s.jitter.range);
        std::uniform_real_distribution<double> probd(0.0, 1.0);
        if (probd(m_rng) <= s.jitter.prob) raw += ud(m_rng);
    }

    // EMA smoothing
    double a = m_globalSmoothingAlpha;
    double next = m_prevInterval * (1.0 - a) + raw * a;

    // clamp
    if (next < m_globalMinInterval) next = m_globalMinInterval;
    if (next > m_globalMaxInterval) next = m_globalMaxInterval;

    m_prevInterval = next;
    std::cout << "Next mob will spawn in " << next << "sec" << std::endl;  // TEST
    return next;
}
