#pragma once
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Independent of rendering/game state so the entire progression can be audited.
class EncounterPlan {
public:
    struct Enemy { std::string type, rarity; double cost; double strength = 1; };
    struct Plan {
        std::string theme;
        bool bossRound = false;
        double budget = 0, spent = 0;
        std::vector<Enemy> enemies;
    };

    EncounterPlan(const nlohmann::json& config, const nlohmann::json& mobs)
        : m_config(config) {
        auto positive = [](double n) { return std::isfinite(n) && n > 0; };
        if (!positive(config.at("spawn_seconds")) || !positive(config.at("prepare_seconds")) ||
            config.at("max_mob").get<int>() < 1 || config.at("max_spawns").get<int>() < 2 ||
            config.at("max_spawns").get<int>() > 200 || config.at("boss_every").get<int>() < 2)
            throw std::invalid_argument("Invalid encounter timing/counts");
        double normal = config.at("normal_max_share"), boss = config.at("boss_max_share");
        if (!positive(normal) || normal > boss || boss >= 1 || !positive(boss))
            throw std::invalid_argument("Invalid encounter enemy budget shares");
        if (config.at("max_spawns").get<int>() < std::ceil(1.0 / normal))
            throw std::invalid_argument("max_spawns is too small for normal_max_share");
        int previousWave = 0;
        double previousBudget = 0;
        for (const auto& a : config.at("progression")) {
            int wave = a.at("wave"); double budget = a.at("budget");
            if (wave <= previousWave || !positive(budget) || budget < previousBudget)
                throw std::invalid_argument("Encounter anchors must increase");
            m_anchors.push_back({wave, budget}); previousWave = wave; previousBudget = budget;
        }
        if (m_anchors.empty() || m_anchors.front().first != 1 || config.at("themes").empty())
            throw std::invalid_argument("Encounters require wave 1 and themes");
        const auto& types = mobs.at("types");
        const auto& bee = types.at("bee").at("rarities");
        for (const auto& [type, multiplierJson] : config.at("enemies").items()) {
            double multiplier = multiplierJson;
            if (!positive(multiplier)) throw std::invalid_argument("Invalid enemy cost multiplier");
            for (const auto& [rarity, stats] : types.at(type).at("rarities").items()) {
                const auto& reference = bee.at(rarity);
                double hp = stats.at("hp"), speed = stats.at("speed");
                double evasion = stats.value("attribs", nlohmann::json::object()).value("evasion", 0.0);
                double armor = stats.at("armor"), damage = stats.at("damage");
                if (!positive(hp) || !positive(speed) || evasion < 0 || evasion >= 1 ||
                    armor < 0 || damage < 0)
                    throw std::invalid_argument("Invalid encounter mob stats");
                // Common bee ~= 1 point. Armor is relative to same-tier damage;
                // type multipliers account for projectiles, offspring and special movement.
                double cost = hp / 37.0 * std::max(0.5, speed / 1.7) / (1 - evasion);
                cost *= 1 + armor / std::max(1.0, reference.at("damage").get<double>());
                cost *= 0.75 + 0.25 * std::sqrt(damage / std::max(1.0, reference.at("damage").get<double>()));
                cost *= multiplier;
                if (!positive(cost)) throw std::invalid_argument("Invalid encounter enemy cost");
                m_enemies.push_back({type, rarity, cost});
            }
        }
        for (const auto& theme : config.at("themes")) {
            if (theme.at("name").get<std::string>().empty()) throw std::invalid_argument("Empty theme name");
            for (const auto& type : theme.at("favor"))
                if (!config.at("enemies").contains(type.get<std::string>()))
                    throw std::invalid_argument("Unknown theme enemy");
        }
        if (m_enemies.empty()) throw std::invalid_argument("Empty encounter enemy catalog");
    }

    double budget(int wave) const {
        for (size_t i = 1; i < m_anchors.size(); ++i) {
            if (wave <= m_anchors[i].first) {
                auto [w0, b0] = m_anchors[i-1]; auto [w1, b1] = m_anchors[i];
                double t = std::clamp(double(wave-w0)/(w1-w0), 0.0, 1.0);
                return std::clamp(b0 * std::pow(b1 / b0, t), b0, b1);
            }
        }
        return m_anchors.back().second;
    }

    Plan build(int wave, unsigned seed) const {
        if (wave < 1) throw std::invalid_argument("Invalid encounter number");
        std::seed_seq sequence{seed, static_cast<unsigned>(wave)};
        std::mt19937 rng(sequence);
        Plan plan;
        plan.budget = budget(wave);
        const auto& themes = m_config.at("themes");
        const auto& theme = themes[(wave-1) % themes.size()];
        bool bossRound = wave % m_config.at("boss_every").get<int>() == 0;
        plan.bossRound = bossRound;
        plan.theme = bossRound ? "Boss" : theme.at("name").get<std::string>();
        double remaining = plan.budget;
        if (bossRound) {
            const Enemy* boss = nullptr;
            for (const auto& e : m_enemies)
                if (e.cost <= plan.budget * m_config.at("boss_max_share").get<double>() &&
                    (!boss || e.cost > boss->cost)) boss = &e;
            if (boss) { plan.enemies.push_back(*boss); remaining -= boss->cost; }
        }
        std::vector<Enemy> pool;
        double highest = 0;
        for (const auto& e : m_enemies)
            if (e.rarity != "super" && e.cost <= plan.budget * m_config.at("normal_max_share").get<double>())
                highest = std::max(highest, e.cost);
        // Retire cheap tiers instead of flooding the map with hundreds of them.
        double floor = std::min(plan.budget / m_config.at("max_spawns").get<int>(), highest * 0.5);
        for (const auto& e : m_enemies)
            if (e.rarity != "super" && e.cost >= floor && e.cost <= highest) pool.push_back(e);
        while (plan.enemies.size() < m_config.at("max_spawns").get<size_t>()) {
            std::vector<double> weights;
            double total = 0;
            for (const auto& e : pool) {
                bool favored = std::find(theme.at("favor").begin(), theme.at("favor").end(), e.type) != theme.at("favor").end();
                double weight = e.cost <= remaining ? (favored ? 3.0 : 1.0) : 0.0;
                weights.push_back(weight); total += weight;
            }
            if (total == 0) break;
            const auto& e = pool[std::discrete_distribution<size_t>(weights.begin(), weights.end())(rng)];
            plan.enemies.push_back(e); remaining -= e.cost;
        }
        if (plan.enemies.empty()) throw std::invalid_argument("Encounter budget cannot afford an enemy");
        // Spend the full budget even between tiers with a large stat gap. Keep
        // composition/count choices, then give regular enemies an equal threat
        // allowance. HP/loot scale linearly; damage is scaled more gently by Mob.
        size_t firstNormal = bossRound ? 1 : 0;
        if (plan.enemies.size() == firstNormal)
            throw std::invalid_argument("Encounter needs affordable regular enemies");
        double normalBudget = plan.budget;
        if (bossRound) {
            double allowance = plan.budget * m_config.at("boss_max_share").get<double>();
            plan.enemies.front().strength = allowance / plan.enemies.front().cost;
            plan.enemies.front().cost = allowance;
            normalBudget -= allowance;
        }
        size_t required = static_cast<size_t>(std::ceil(normalBudget /
            (plan.budget * m_config.at("normal_max_share").get<double>())));
        while (plan.enemies.size() - firstNormal < required)
            plan.enemies.push_back(plan.enemies[firstNormal]);
        double allowance = normalBudget / (plan.enemies.size() - firstNormal);
        for (size_t i = firstNormal; i < plan.enemies.size(); ++i) {
            plan.enemies[i].strength = allowance / plan.enemies[i].cost;
            plan.enemies[i].cost = allowance;
        }
        plan.spent = plan.budget;
        return plan;
    }

private:
    nlohmann::json m_config;
    std::vector<std::pair<int, double>> m_anchors;
    std::vector<Enemy> m_enemies;
};
