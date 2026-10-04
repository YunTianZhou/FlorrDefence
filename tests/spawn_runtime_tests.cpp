#include <iostream>
#include <stdexcept>
#include "AssetManager.hpp"
#include "Map.hpp"
#include "PlayerStateDisplayer.hpp"
#include "SpriteCollisionManager.hpp"

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    try {
        loadConstants();
        AssetManager::load();
        SpriteCollisionManager::load();
        SharedInfo info;
        info.init();
        info.dt = sf::seconds(0.125f);
        SpawnManager spawner(&info);
        std::list<std::unique_ptr<Mob>> mobs;
        auto initial = spawner.save();
        require(initial.at("mode") == "encounters", "Wrong default mode");
        require(initial.at("wave") == 1 && initial.at("preparing"), "Wrong initial state");
        spawner.update(mobs);
        require(mobs.empty(), "Spawned during preparation");
        spawner.startEarly();
        spawner.update(mobs);
        spawner.update(mobs);
        require(mobs.size() == 1, "Early start did not spawn");
        auto inProgress = spawner.save();
        SharedInfo otherInfo;
        otherInfo.init(); otherInfo.dt = info.dt;
        SpawnManager restored(&otherInfo);
        restored.restore(inProgress);
        require(restored.save() == inProgress, "Mid-wave state lost on restore");
        std::list<std::unique_ptr<Mob>> otherMobs;
        otherMobs.push_back(Mob::create(&otherInfo, mobs.front()->getMob(), otherMobs));
        info.playerState.level = 250;
        for (int i = 0; i < 450; ++i) {
            spawner.update(mobs); restored.update(otherMobs);
            require(spawner.save() == restored.save(), "XP/level or reload changed spawning");
            require(mobs.size() == otherMobs.size(), "Restored spawn count changed");
        }
        require(spawner.save().at("wave") == 1, "Advanced before clear");
        mobs.clear();
        spawner.update(mobs);
        require(spawner.save().at("wave") == 2 && spawner.save().at("preparing"), "Did not advance after clear");
        auto preparing = spawner.save();
        restored.restore(preparing);
        require(restored.save() == preparing, "Preparation state lost");

        // A full live list pauses the timer; clearing it cannot cause a catch-up burst.
        spawner.startEarly(); spawner.update(mobs);
        while (mobs.size() < info.mobLimit)
            mobs.push_back(Mob::create(&info, {"common", "bee"}, mobs));
        auto capped = spawner.save();
        for (int i = 0; i < 100; ++i) spawner.update(mobs);
        require(spawner.save() == capped, "Live cap advanced spawning");
        AntEggMob egg(&info, {"common", "ant_egg"}, mobs);
        HornetMob hornet(&info, {"common", "hornet"}, mobs);
        AntQueenMob queen(&info, {"common", "ant_queen"}, mobs);
        info.dt = sf::seconds(1.f);
        for (int i = 0; i < 100; ++i) {
            egg.onDead(); hornet.update(); queen.update();
            require(mobs.size() == info.mobLimit, "Offspring bypassed live cap");
        }
        info.dt = sf::seconds(0.125f);
        mobs.clear(); spawner.update(mobs);
        require(mobs.size() == 1, "Catch-up burst after cap");

        // Exercise Map's real record boundary, including saved live mobs and wave jumps.
        Map map(&info);
        map.getMobs().push_back(Mob::create(&info, {"common", "bee"}, map.getMobs()));
        auto& scaled = *map.getMobs().front();
        scaled.setEncounterStrength(4);
        scaled.setEncounterBoss(true);
        require(scaled.getHp() == 4 * MOB_ATTRIBS.at("bee")["common"].hp, "HP scaling failed");
        require(scaled.getDamage() == 2 * MOB_ATTRIBS.at("bee")["common"].damage, "Damage scaling failed");
        require(scaled.getAttribs().coinDrop == 4 * MOB_ATTRIBS.at("bee")["common"].coinDrop, "Reward scaling failed");
        scaled.hit(10, DamageType::Lightning);
        json savedMap = map;
        Map loadedMap(&otherInfo);
        savedMap.get_to(loadedMap);
        require(json(loadedMap) == savedMap, "Map record did not round-trip");
        require(loadedMap.getMobs().front()->isEncounterBoss(), "Boss marker lost on load");
        auto oldRecord = savedMap;
        oldRecord.erase("spawner");
        bool oldRejected = false;
        try { oldRecord.get_to(loadedMap); } catch (const std::invalid_argument&) { oldRejected = true; }
        require(oldRejected, "Record without encounter progress accepted");
        require(loadedMap.getMapInfo().findSquareAndPlace({"common", "basic"}), "Debug defense setup failed");
        auto towersBeforeSkip = json(loadedMap.getMapInfo());
        loadedMap.getPetals().push_back(ShootPetal::create(&otherInfo, {"common", "basic"},
            {0.f, 0.f}, loadedMap.getMobs().cbegin()));
        auto beforeSkip = otherInfo.playerState;
        loadedMap.advanceWaves(10);
        auto skipped = json(loadedMap);
        require(skipped.at("spawner").at("wave") == 11, "Debug wave jump failed");
        require(skipped.at("spawner").at("preparing") && skipped.at("spawner").at("spawned") == 0,
            "Debug jump did not reset preparation/queue");
        require(loadedMap.getMobs().empty(), "Debug jump kept enemies");
        require(json(loadedMap.getMapInfo()) == towersBeforeSkip, "Debug jump changed towers");
        require(loadedMap.getPetals().size() == 1, "Debug jump removed friendly petals");
        loadedMap.getPetals().front()->update(); // A removed homing target must not be dereferenced.
        require(otherInfo.playerState.coin == beforeSkip.coin && otherInfo.playerState.xp == beforeSkip.xp &&
            otherInfo.playerState.level == beforeSkip.level && otherInfo.playerState.talent == beforeSkip.talent,
            "Debug wave jump awarded resources or changed level");
        Map skippedReload(&info);
        skipped.get_to(skippedReload);
        require(json(skippedReload) == skipped, "Debug jump did not survive save/load");
        loadedMap.advanceWaves(0);
        require(json(loadedMap) == skipped, "Zero wave jump changed state");
        auto malformed = inProgress;
        malformed["spawned"] = 201;
        bool rejected = false;
        try { restored.restore(malformed); } catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Invalid saved progress accepted");
        info.encounterStatus = "Wave 90 / Boss - 39 pending, 100 alive";
        PlayerStateDisplayer display(info);
        display.update();
        sf::RenderTexture preview({1700, 240});
        preview.clear(LIGHT_COLORS.at("wood"));
        preview.draw(display);
        preview.display();
        require(preview.getTexture().copyToImage().saveToFile("encounter-status.png"), "Could not render status preview");
        // Smoke-test real movement, collisions and damage with the starting cards.
        // This deliberately does not claim to simulate purchasing or player strategy.
        SharedInfo battleInfo;
        battleInfo.init();
        Map battle(&battleInfo);
        for (int i = 0; i < 5; ++i)
            require(battle.getMapInfo().findSquareAndPlace({"common", "basic"}), "Starting card placement failed");
        battle.getMapInfo().updateTowerBuff();
        battle.startEncounterEarly();
        int frames = 0;
        for (; frames < 1440 && battleInfo.playerState.isAlive(); ++frames) {
            battleInfo.dt = sf::seconds(0.125f);
            battleInfo.playerState.update();
            battle.update();
            if (json(battle).at("spawner").at("wave") != 1) break;
        }
        std::cout << "Starting-defense smoke: " << frames * 0.125 << "s, HP "
            << battleInfo.playerState.hp << ", wave " << json(battle).at("spawner").at("wave") << '\n';
        std::cout << "PASS: early start, XP independence, clear gate, cap, save/load, debug wave jumps, old-record rejection\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
