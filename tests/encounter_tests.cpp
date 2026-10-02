#include <fstream>
#include <iostream>
#include <stdexcept>
#include <limits>
#include "EncounterPlan.hpp"

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    try {
        nlohmann::json config, mobs;
        std::ifstream("res/config/mob_spawn_config.json") >> config;
        std::ifstream("res/config/mob_attribs.json") >> mobs;
        EncounterPlan rules(config, mobs);
        for (unsigned seed = 0; seed < 64; ++seed) {
            for (int wave = 1; wave <= 120; ++wave) {
                auto plan = rules.build(wave, seed);
                auto repeat = rules.build(wave, seed);
                require(!plan.enemies.empty() && plan.enemies.size() <= config.at("max_spawns").get<size_t>(), "Invalid count");
                require(plan.spent <= plan.budget + 1e-6, "Budget exceeded");
                require(plan.spent >= plan.budget * 0.999, "Unused budget creates a plateau");
                require(plan.enemies.size() == repeat.enemies.size(), "Unstable seeded count");
                double spent = 0;
                for (size_t i = 0; i < plan.enemies.size(); ++i) {
                    auto e = plan.enemies[i];
                    bool boss = wave % config.at("boss_every").get<int>() == 0 && i == 0;
                    double share = config.at(boss ? "boss_max_share" : "normal_max_share");
                    require(e.cost <= plan.budget * share + 1e-6, "Single mob exceeds share");
                    require(boss || e.rarity != "super", "Unscheduled Super mob");
                    require(e.type == repeat.enemies[i].type && e.rarity == repeat.enemies[i].rarity, "Unstable seeded plan");
                    require(e.strength == repeat.enemies[i].strength && e.strength > 0, "Invalid strength");
                    const auto& stats = mobs.at("types").at(e.type).at("rarities").at(e.rarity);
                    require(stats.at("hp").get<double>() * e.strength < std::numeric_limits<int>::max() / 2,
                        "Default progression would clamp HP");
                    require(stats.at("xp_drop").get<double>() * e.strength < std::numeric_limits<int>::max() / 2,
                        "Default progression would clamp XP");
                    spent += e.cost;
                }
                require(std::abs(spent - plan.spent) < plan.budget * 1e-10, "Incorrect spending");
                if (wave > 1) {
                    require(plan.budget >= rules.budget(wave-1), "Budget decreased");
                    require(plan.budget / rules.budget(wave-1) <= 1.26, "Budget cliff");
                }
                if (seed == 0 && (wave <= 5 || wave % 5 == 0))
                    std::cout << "Wave " << wave << ": " << plan.theme << ", " << plan.enemies.size()
                        << " mobs, " << int(100 * plan.spent / plan.budget) << "% budget used, first "
                        << plan.enemies.front().rarity << ' ' << plan.enemies.front().type
                        << " x" << plan.enemies.front().strength << '\n';
            }
        }
        auto invalid = config;
        invalid["progression"][1]["wave"] = 1;
        bool rejected = false;
        try { EncounterPlan bad(invalid, mobs); } catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Overlapping anchors accepted");
        std::cout << "PASS: 7,680 seeded encounters, budget bounds, progression, determinism, validation\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
