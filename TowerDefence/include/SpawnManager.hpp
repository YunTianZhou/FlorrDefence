#pragma once

#include <random>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <SFML/System.hpp>
#include <memory>
#include <functional>

#include "Mob.hpp"

enum class Rarity { Common, Unusual, Rare, Epic, Legendary };

struct SpawnRecord {
    std::string type;
    float baseWeight;
    int minLevel = 1;
    int maxLevel = 9999;
};

struct SpawnConfig {
    float waveInterval = 20.f;           // seconds between each wave
    float baseSpawnInterval = 1.0f;      // seconds between spawn attempts (base)
    float spawnIntervalMin = 0.12f;      // minimum interval
    float spawnIntervalDecay = 0.999f;   // multiplicative decay per wave
    int maxMobs = 500;                   // the limit of mobs
};

class SpawnManager {
public:
    SpawnManager(SharedInfo& info)
        : m_info(info)
    {
        std::random_device rd;
        m_rng.seed(rd());
        std::uniform_real_distribution<float> d(0.f, 2.f * 3.14159265f);
        m_phase = d(m_rng);

        m_spawnAccumulator = sf::Time::Zero;
        m_timeSeconds = 0.f;
        m_nextSpawnInterval = -1.f;
        m_lastLevel = -1;
    }

    void addSpawnRecord(Rarity r, SpawnRecord rec) {
        m_pool.push_back({ r, std::move(rec) });
    }

    void update(sf::Time dt, std::list<std::unique_ptr<Mob>>& mobList) {
        m_spawnAccumulator += dt;
        m_timeSeconds += dt.asSeconds();

        int level = m_info.playerState.level;

        if (m_nextSpawnInterval < 0.f || level != m_lastLevel) {
            m_nextSpawnInterval = computeNextInterval(level);
            m_lastLevel = level;
        }

        while (m_spawnAccumulator.asSeconds() >= m_nextSpawnInterval) {
            m_spawnAccumulator -= sf::seconds(m_nextSpawnInterval);
            if ((int)mobList.size() >= m_config.maxMobs) break;

            std::vector<float> weights; weights.reserve(m_pool.size());
            std::vector<size_t> idxs; idxs.reserve(m_pool.size());

            for (size_t i = 0; i < m_pool.size(); i++) {
                const auto& e = m_pool[i];
                const SpawnRecord& r = e.record;
                if (level < r.minLevel || level > r.maxLevel) continue;
                float rarityMult = rarityMultiplier(e.rarity, level);
                float w = r.baseWeight * rarityMult;
                if (w > 0.0f) { weights.push_back(w); idxs.push_back(i); }
            }

            if (weights.empty()) {
                m_nextSpawnInterval = -1.f;
                break;
            }

            size_t pick = weightedIndex(weights);
            const auto& chosen = m_pool[idxs[pick]];

            MobInfo minfo(rarityToString(chosen.rarity), chosen.record.type);
            auto mob = Mob::create(m_info, minfo, mobList);
            mobList.push_back(std::move(mob));

            m_nextSpawnInterval = computeNextInterval(level);
        }
    }

    SpawnConfig& config() { return m_config; }

private:
    struct PoolEntry { Rarity rarity; SpawnRecord record; };
    SharedInfo& m_info;
    std::vector<PoolEntry> m_pool;
    std::mt19937 m_rng;
    sf::Time m_spawnAccumulator;
    SpawnConfig m_config;

    // new internal state
    float m_timeSeconds = 0.f;
    float m_phase = 0.f;
    float m_nextSpawnInterval = -1.f;
    int m_lastLevel = -1;

    const float m_intervalOscillationAmplitude = 0.28f;
    const float m_intervalOscillationFrequency = 0.5f;
    const float m_intervalJitterFraction = 0.33f;

    static float rarityMultiplier(Rarity r, int level) {
        switch (r) {
        case Rarity::Common:    return std::clamp(1.0f - level * 0.01f, 0.15f, 1.0f);
        case Rarity::Unusual:   return std::clamp(0.6f + level * 0.008f, 0.1f, 1.6f);
        case Rarity::Rare:      return std::clamp(0.25f + level * 0.01f, 0.05f, 1.3f);
        case Rarity::Epic:      return std::clamp(0.08f + level * 0.007f, 0.0f, 1.0f);
        case Rarity::Legendary: return std::clamp(0.02f + level * 0.003f, 0.0f, 0.9f);
        }
        return 1.0f;
    }

    float computeNextInterval(int level) {
        float baseInterval = std::max(m_config.spawnIntervalMin,
            m_config.baseSpawnInterval * std::pow(m_config.spawnIntervalDecay, (float)(level - 1)));

        // Oscillation sampled at the moment we schedule the spawn (not every frame)
        float osc = 1.f + m_intervalOscillationAmplitude * std::sin(m_timeSeconds * m_intervalOscillationFrequency + m_phase);

        // Jitter sampled ONCE per spawn
        std::uniform_real_distribution<float> jitterDist(-m_intervalJitterFraction, m_intervalJitterFraction);
        float jitter = jitterDist(m_rng);

        float interval = baseInterval * osc * (1.f + jitter);

        float upperBound = std::max(baseInterval * 2.f, m_config.spawnIntervalMin);
        return std::clamp(interval, m_config.spawnIntervalMin, upperBound);
    }

    std::string rarityToString(Rarity r) const {
        switch (r) {
        case Rarity::Common: return "common";
        case Rarity::Unusual: return "unusual";
        case Rarity::Rare: return "rare";
        case Rarity::Epic: return "epic";
        case Rarity::Legendary: return "legendary";
        }
        return "common";
    }

    size_t weightedIndex(const std::vector<float>& weights) {
        float sum = std::accumulate(weights.begin(), weights.end(), 0.f);
        std::uniform_real_distribution<float> dist(0.f, sum);
        float x = dist(m_rng);
        float cur = 0.f;
        for (size_t i = 0; i < weights.size(); i++) {
            cur += weights[i];
            if (x <= cur) return i;
        }
        return weights.size() - 1;
    }
};
