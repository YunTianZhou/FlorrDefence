#include "SpawnManager.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <nlohmann/json.hpp>

#include "Mob.hpp"

SpawnManager::SpawnManager(SharedInfo* info)
    : m_info(info)
{
    std::random_device rd;
    m_seed = rd();
    m_rng.seed(m_seed);
    load();
}

void SpawnManager::load() {
    std::ifstream config("res/config/mob_spawn_config.json");
    config >> m_config;
    std::string mode = m_config.at("mode");
    if (mode != "legacy" && mode != "encounters")
        throw std::invalid_argument("Unknown spawn mode");
    m_legacy = mode == "legacy";
    if (m_legacy) { loadLegacy(); return; }
    std::ifstream mobs("res/config/mob_attribs.json");
    nlohmann::json stats;
    mobs >> stats;
    m_rules = std::make_unique<EncounterPlan>(m_config, stats);
    m_maxMob = m_config.at("max_mob").get<size_t>();
    m_info->mobLimit = m_maxMob;
    prepareEncounter();
}

void SpawnManager::loadLegacy() {
    std::ifstream ifs("res/config/mob_spawn_legacy.json");
    if (!ifs.is_open())
        throw std::runtime_error("Failed to open mob_spawn_legacy.json");

    nlohmann::json j;
    ifs >> j;

    auto gj = j["global"];
    if (gj.contains("min_interval")) m_globalMinInterval = gj["min_interval"].get<double>();
    if (gj.contains("max_interval")) m_globalMaxInterval = gj["max_interval"].get<double>();
    if (gj.contains("smoothing_alpha")) m_globalSmoothingAlpha = gj["smoothing_alpha"].get<double>();

    m_maxMob = j["max_mob"].get<int>();
    m_info->mobLimit = std::numeric_limits<size_t>::max();
    m_info->encounterStatus = "Legacy spawning (player level)";

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
    if (!m_legacy) { updateEncounter(mobList); return; }
    m_spawnTimer += m_info->dt;
    m_globalTimer += m_info->dt;

    if (mobList.size() >= m_maxMob) return;
    if (m_spawnTimer.asSeconds() < m_nextInterval) return;
    m_spawnTimer = sf::Time::Zero;

    int playerLevel = m_info->playerState.level;
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

void SpawnManager::prepareEncounter() {
    auto plan = m_rules->build(m_wave, m_seed);
    m_theme = plan.theme;
    m_bossRound = plan.bossRound;
    m_queue.clear();
    m_strengths.clear();
    for (const auto& e : plan.enemies) {
        m_queue.push_back({e.rarity, e.type});
        m_strengths.push_back(e.strength);
    }
    m_spawned = 0;
    m_preparing = true;
    m_remaining = m_config.at("prepare_seconds");
    m_interval = m_config.at("spawn_seconds").get<double>() / m_queue.size();
    updateStatus(0);
}

void SpawnManager::startEarly() {
    if (!m_legacy && m_preparing) m_remaining = 0;
}

void SpawnManager::updateStatus(size_t alive) {
    m_info->encounterStatus = "Wave " + std::to_string(m_wave) + " / " + m_theme + " - ";
    if (m_preparing)
        m_info->encounterStatus += std::to_string(static_cast<int>(std::ceil(m_remaining))) + "s (N: start)";
    else
        m_info->encounterStatus += std::to_string(m_queue.size() - m_spawned) + " pending, " + std::to_string(alive) + " alive";
}

void SpawnManager::updateEncounter(std::list<std::unique_ptr<Mob>>& mobs) {
    double dt = std::max(0.0f, m_info->dt.asSeconds());
    if (m_preparing) {
        m_remaining = std::max(0.0, m_remaining - dt);
        if (m_remaining == 0) m_preparing = false;
    } else if (m_spawned == m_queue.size()) {
        if (mobs.empty()) { ++m_wave; prepareEncounter(); }
    } else if (mobs.size() < m_maxMob) {
        double rate = std::max(0.01f, m_info->playerState.buff.mob_spawn_rate.apply(1.f));
        m_remaining -= dt * rate;
        if (m_remaining <= 0) {
            // No burst of overdue spawns when the live-mob cap clears.
            auto mob = Mob::create(m_info, m_queue[m_spawned], mobs);
            mob->setEncounterStrength(m_strengths[m_spawned]);
            mob->setEncounterBoss(m_bossRound && m_spawned == 0);
            mobs.push_back(std::move(mob));
            ++m_spawned;
            m_remaining = m_interval;
        }
    }
    updateStatus(mobs.size());
}

nlohmann::json SpawnManager::save() const {
    std::ostringstream rng;
    rng << m_rng;
    if (m_legacy)
        return {{"mode", "legacy"}, {"rng", rng.str()},
            {"spawn_timer", m_spawnTimer.asSeconds()}, {"global_timer", m_globalTimer.asSeconds()},
            {"next_interval", m_nextInterval}, {"previous_interval", m_prevInterval}};
    return {{"mode", "encounters"}, {"version", 1}, {"wave", m_wave}, {"seed", m_seed},
        {"queue", m_queue}, {"strengths", m_strengths}, {"spawned", m_spawned}, {"preparing", m_preparing},
        {"remaining", m_remaining}, {"interval", m_interval}, {"theme", m_theme}, {"boss_round", m_bossRound}};
}

void SpawnManager::restore(const nlohmann::json& state) {
    // Old records retain level-based spawning; new records retain their mode.
    std::string mode = state.is_null() ? "legacy" : state.at("mode").get<std::string>();
    if (mode == "legacy") {
        m_legacy = true;
        loadLegacy();
        if (!state.is_null()) {
            std::istringstream rng(state.at("rng").get<std::string>());
            if (!(rng >> m_rng)) throw std::invalid_argument("Invalid saved spawn RNG");
            m_spawnTimer = sf::seconds(state.at("spawn_timer").get<float>());
            m_globalTimer = sf::seconds(state.at("global_timer").get<float>());
            m_nextInterval = state.at("next_interval");
            m_prevInterval = state.at("previous_interval");
        }
        return;
    }
    if (mode != "encounters" || state.at("version") != 1)
        throw std::invalid_argument("Unsupported encounter record");
    if (!m_rules) {
        std::ifstream mobs("res/config/mob_attribs.json");
        nlohmann::json stats; mobs >> stats;
        m_rules = std::make_unique<EncounterPlan>(m_config, stats);
    }
    m_legacy = false;
    m_maxMob = m_config.at("max_mob").get<size_t>();
    m_info->mobLimit = m_maxMob;
    m_wave = state.at("wave"); m_seed = state.at("seed");
    m_queue = state.at("queue").get<std::vector<MobInfo>>();
    m_strengths = state.at("strengths").get<std::vector<double>>();
    m_spawned = state.at("spawned"); m_preparing = state.at("preparing");
    m_remaining = state.at("remaining"); m_interval = state.at("interval");
    m_theme = state.at("theme");
    m_bossRound = state.at("boss_round");
    if (m_wave < 1 || m_wave >= std::numeric_limits<int>::max()-1 || m_queue.empty() ||
        m_queue.size() > 200 || m_strengths.size() != m_queue.size() || m_spawned > m_queue.size() || !std::isfinite(m_remaining) ||
        m_remaining < 0 || !std::isfinite(m_interval) || m_interval <= 0 || (m_preparing && m_spawned != 0))
        throw std::invalid_argument("Invalid saved encounter progress");
    for (double strength : m_strengths)
        if (!std::isfinite(strength) || strength <= 0 || strength > 1000000)
            throw std::invalid_argument("Invalid saved encounter strength");
    for (const auto& mob : m_queue)
        if (!m_config.at("enemies").contains(mob.type) ||
            !MOB_ATTRIBS.at(mob.type).rarities.contains(mob.rarity))
            throw std::invalid_argument("Invalid saved encounter mob");
    updateStatus(0);
}

double SpawnManager::computeNextInterval(const Stage& s, int level) {
    double t = m_globalTimer.asSeconds();
    double raw = s.base_interval;
    double rate = m_info->playerState.buff.mob_spawn_rate.apply(1.f);

    // Apply linear decrease per level (scale_per_level usually negative)
    int levelOffset = level - s.min_level;
    raw += s.scale_per_level * (double)(levelOffset);

    // Oscillator (sine)
    if (s.oscillator.enabled && s.oscillator.period > 0.0) {
        double phase = (2.0 * 3.14159265358979323846 * t) / s.oscillator.period;
        raw += std::sin(phase) * s.oscillator.amplitude;
    }

    // Jitter (instant)
    if (s.jitter.range > 0.0) {
        std::uniform_real_distribution<double> ud(-s.jitter.range, s.jitter.range);
        std::uniform_real_distribution<double> probd(0.0, 1.0);
        if (probd(m_rng) <= s.jitter.prob) raw += ud(m_rng);
    }

    // Apply spawn rate
    if (rate > 0.0)
        raw /= rate;

    // EMA smoothing
    double a = m_globalSmoothingAlpha;
    double next = m_prevInterval * (1.0 - a) + raw * a;

    // clamp
    if (next < m_globalMinInterval) next = m_globalMinInterval;
    if (next > m_globalMaxInterval) next = m_globalMaxInterval;

    m_prevInterval = next;
    return next;
}
