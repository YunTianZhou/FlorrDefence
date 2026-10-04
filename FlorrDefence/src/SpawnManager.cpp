#include "SpawnManager.hpp"

#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>

#include "Mob.hpp"

SpawnManager::SpawnManager(SharedInfo* info)
    : m_info(info)
{
    std::random_device rd;
    m_seed = rd();
    load();
}

void SpawnManager::load() {
    std::ifstream config("res/config/mob_spawn_config.json");
    config >> m_config;
    std::ifstream mobs("res/config/mob_attribs.json");
    nlohmann::json stats;
    mobs >> stats;
    m_rules = std::make_unique<EncounterPlan>(m_config, stats);
    m_maxMob = m_config.at("max_mob").get<size_t>();
    m_info->mobLimit = m_maxMob;
    prepareEncounter();
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
    if (m_preparing) m_remaining = 0;
}

void SpawnManager::advanceWaves(int amount) {
    if (amount <= 0) return;
    m_wave += std::min(amount, std::numeric_limits<int>::max() - 2 - m_wave);
    prepareEncounter();
}

void SpawnManager::updateStatus(size_t alive) {
    m_info->encounterStatus = "Wave " + std::to_string(m_wave) + " / " + m_theme + " - ";
    if (m_preparing)
        m_info->encounterStatus += std::to_string(static_cast<int>(std::ceil(m_remaining))) + "s (N: start)";
    else
        m_info->encounterStatus += std::to_string(m_queue.size() - m_spawned) + " pending, " + std::to_string(alive) + " alive";
}

void SpawnManager::update(std::list<std::unique_ptr<Mob>>& mobs) {
    double dt = std::max(0.0f, m_info->dt.asSeconds());
    if (m_preparing) {
        m_remaining = std::max(0.0, m_remaining - dt);
        if (m_remaining == 0) m_preparing = false;
    } else if (m_spawned == m_queue.size()) {
        if (mobs.empty()) advanceWaves(1);
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
    return {{"mode", "encounters"}, {"version", 1}, {"wave", m_wave}, {"seed", m_seed},
        {"queue", m_queue}, {"strengths", m_strengths}, {"spawned", m_spawned}, {"preparing", m_preparing},
        {"remaining", m_remaining}, {"interval", m_interval}, {"theme", m_theme}, {"boss_round", m_bossRound}};
}

void SpawnManager::restore(const nlohmann::json& state) {
    if (!state.is_object() || state.value("mode", std::string{}) != "encounters" ||
        state.value("version", 0) != 1)
        throw std::invalid_argument("Record has no supported encounter progress");
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
