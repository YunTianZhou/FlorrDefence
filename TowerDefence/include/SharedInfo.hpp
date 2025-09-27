#pragma once
#include <string>
#include <map>
#include <set>
#include <array>
#include <SFML/Graphics.hpp>
#include "DraggedCard.hpp"
#include "Buff.hpp"
#include "Constants.hpp"

class DefencePetal;

class BackpackInfo {
public:
    int getCount(const CardInfo& card) const;
    int getRarityCount(const std::string& rarity) const;
    int getTypeCount(const std::string& type) const;
    void add(const CardStackInfo& stack);

private:
    std::map<CardInfo, int> m_count;
    std::map<std::string, int> m_rarityCount;
    std::map<std::string, int> m_typeCount;
};

struct Counter {
    std::map<CardInfo, int> tower;
    std::map<CardInfo, int> petal;
    std::map<MobInfo, int> mob;
};

struct PlayerState {
    struct Accum {
        float heal = 0.f;
        float shield = 0.f;
    };

    int hpLimit = 0;
    int hp = 0;
    int shield = 0;
    int xp = 0;
    int bodyDamage = 0;
    int level = 0;
    int coin = 0;
    int talent = 0;
    Accum acc;
    BackpackInfo backpack;
    Buff buff;
    BuffManager buffManager;
    std::set<std::string> boughtUniques;

    void init();
    int calcRequiredXp() const;

    int getBodyDamage() const;
    int getHpLimit() const;
    void hit(int damage);
    void heal(float amount);
    void addShield(float amount);
    void addXp(int amount);
    void addCoin(int amount);

    void updateLevel();
    void update();

    void applyHealBuff(sf::Time dt);
};

struct InputInfo {
    bool mouseLeftButton = false;

    bool keyG = false;
    bool keyH = false;
    bool keyShift = false;

    void update();
};

struct SharedInfo {
    sf::Vector2f mouseWorldPosition;
    InputInfo input;
    sf::Time time;
    sf::Time dt;
    PlayerState playerState;
    std::array<std::array<const DefencePetal*, 10>, 11> defencePetalMap = {};
    Counter counter;
    
    std::optional<DraggedCard> draggedCard;
    sf::Clock timeClock;
    sf::Clock dtClock;

    void init();
    bool update(const sf::RenderWindow& window);
};
