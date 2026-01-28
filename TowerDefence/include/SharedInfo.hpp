#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <array>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include "DraggedCard.hpp"
#include "Buff.hpp"
#include "Constants.hpp"

using nlohmann::json;

class DefencePetal;

class BackpackInfo {
public:
    int getCount(const CardInfo& card) const;
    int getRarityCount(const std::string& rarity) const;
    int getTypeCount(const std::string& type) const;
    void add(const CardStackInfo& stack);

    friend void to_json(json& j, const BackpackInfo& b);
    friend void from_json(const json& j, BackpackInfo& b);

private:
    std::map<CardInfo, int> m_count;
    std::unordered_map<std::string, int> m_rarityCount;
    std::unordered_map<std::string, int> m_typeCount;
};

struct Counter {
    std::map<CardInfo, int> tower;
    std::map<CardInfo, int> petal;
    std::map<MobInfo, int> mob;
};

inline void to_json(json& j, const BackpackInfo& b) {
    j["cards"] = json::array();

    for (const auto& [card, count] : b.m_count) {
        j["cards"].push_back({
            {"card", card},
            {"count", count}
        });
    }
}

inline void from_json(const json& j, BackpackInfo& b) {
    b = {}; // reset

    for (const auto& e : j.at("cards")) {
        CardInfo card = e.at("card").get<CardInfo>();
        int count = e.at("count").get<int>();
        b.add({ card, count });
    }
}

struct PlayerState {
    struct Accum {
        float heal = 0.f;
        float shield = 0.f;
    };

    int originalHpLimit = 0;
    int prevHpLimit = 0;
    int hpLimit = 0;
    int hp = 0;
    int shield = 0;
    int xp = 0;
    int bodyDamage = 0;
    int level = 0;
    int64_t coin = 0;
    int talent = 0;
    Accum acc;
    BackpackInfo backpack;
    BuffGroup towerBuff;
    BuffGroup talentBuff;
    BuffGroup buff;
    BuffManager buffManager;
    std::set<std::string> boughtUniques;

    void init();
    int calcRequiredXp() const;

    int getBodyDamage() const;
    void hit(int damage);
    void heal(float amount);
    void addShield(float amount);
    void addXp(int amount);
    void addCoin(int64_t amount);

    void updateLevel();
    void update();

    void applyHealValueBuff(sf::Time dt);
};

inline void to_json(json& j, const PlayerState& p) {
    j = {
        { "hpLimit",        p.hpLimit },
        { "hp",             p.hp },
        { "shield",         p.shield },
        { "xp",             p.xp },
        { "bodyDamage",     p.bodyDamage },
        { "level",          p.level },
        { "coin",           p.coin },
        { "talent",         p.talent },
        { "backpack",       p.backpack },
        { "bought_uniques", p.boughtUniques }
    };
}

inline void from_json(const json& j, PlayerState& p) {
    p.init();

    p.hpLimit = j.value("hpLimit", 0);
    p.hp = j.value("hp", 0);
    p.shield = j.value("shield", 0);
    p.xp = j.value("xp", 0);
    p.bodyDamage = j.value("bodyDamage", 0);
    p.level = j.value("level", 0);
    p.coin = j.value("coin", int64_t{ 0 });
    p.talent = j.value("talent", 0);

    if (j.contains("backpack"))
        p.backpack = j.at("backpack").get<BackpackInfo>();
    else
        p.backpack = BackpackInfo{};

    if (j.contains("bought_uniques"))
        p.boughtUniques = j.at("bought_uniques").get<std::set<std::string>>();
    else
        p.boughtUniques.clear();

    p.prevHpLimit = p.hpLimit;
    p.hp = std::clamp(p.hp, 0, p.hpLimit);
}

struct InputInfo {
    bool mouseLeftButton = false;
    bool mouseRightButton = false;

    bool keyG = false;
    bool keyH = false;
    bool keyShift = false;
    bool keyCtrl = false;

    void update();
};

struct SharedInfo {
    sf::Vector2f mouseWorldPosition;
    InputInfo input;
    sf::Time dt;
    PlayerState playerState;
    std::array<std::array<DefencePetal*, 10>, 11> defencePetalMap = {};
    std::array<std::array<bool, 10>, 11> laserMap = {};
    Counter counter;
    
    std::optional<DraggedCard> draggedCard;
    std::optional<CardStackInfo> placeRequest;
    sf::Clock dtClock;
    
    void init();
    bool update(const sf::RenderWindow& window);
};
