#include "SharedInfo.hpp"

// BackpackInfo
int BackpackInfo::getCount(const CardInfo& card) const {
    auto it = m_count.find(card);
    return it != m_count.end() ? it->second : 0;
}

int BackpackInfo::getRarityCount(const std::string& rarity) const {
    auto it = m_rarityCount.find(rarity);
    return it != m_rarityCount.end() ? it->second : 0;
}

int BackpackInfo::getTypeCount(const std::string& type) const {
    auto it = m_typeCount.find(type);
    return it != m_typeCount.end() ? it->second : 0;
}

void BackpackInfo::add(const CardStackInfo& stack) {
    m_count[stack.card] += stack.count;
    m_rarityCount[stack.card.rarity] += stack.count;
    m_typeCount[stack.card.type] += stack.count;
}

// PlayerState
void PlayerState::init() {
    originalHpLimit = prevHpLimit = hpLimit = INIT_STATES.hp;
    hp = INIT_STATES.hp;
    xp = INIT_STATES.xp;
    bodyDamage = INIT_STATES.bodyDamage;
    level = INIT_STATES.level;
    coin = INIT_STATES.coin;
    talent = INIT_STATES.talent;
    for (auto& stack : INIT_STATES.cards)
        backpack.add(stack);
    updateLevel();
}

int PlayerState::calcRequiredXp() const {
    return 100 * (level + 1) * (level + 2) / 2;
}

int PlayerState::getBodyDamage() const {
    return (int)buff.bodyDamage.apply((float)bodyDamage);
}

void PlayerState::hit(int damage) {
    if (shield > 0) {
        int obsorbed = std::min(damage, shield);
        shield -= obsorbed;
        damage -= obsorbed;
    }
    if (damage > 0) {
        hp -= damage;
    }
}

void PlayerState::heal(float amount) {
    acc.heal += amount;
    if (acc.heal < 1.f) return;
    int actuall_amount = int(acc.heal);
    acc.heal -= actuall_amount;

    int heal = std::min(actuall_amount, hpLimit - hp);
    hp += heal;
    hp = std::min(hp, hpLimit);
    actuall_amount -= heal;
    if (actuall_amount > 0) 
        addShield(buff.overheal.apply((float)actuall_amount));
}

void PlayerState::addShield(float amount) {
    acc.shield += amount;
    if (acc.shield >= 1.f) {
        shield += int(acc.shield);
        acc.shield -= int(acc.shield);
    }
}

void PlayerState::addXp(int amount) {
    xp += amount;
    updateLevel();
}

void PlayerState::addCoin(int64_t amount) {
     coin += amount;
}

void PlayerState::updateLevel() {
    int required = 0;
    while (xp >= (required = calcRequiredXp())) {
        level++;
        talent++;
        xp -= required;
    }
}

void PlayerState::update() {
    updateLevel();

    hp = std::min(hp, hpLimit);
    shield = std::max(0, std::min(shield, hp));

    hpLimit = (int)buff.health.apply((float)originalHpLimit);
    if (hpLimit != prevHpLimit) {
        double hpPrecent = (double)hp / prevHpLimit;
        double shieldPrecent = (double)shield / prevHpLimit;
        hp = int(hpLimit * hpPrecent);
        shield = int(hpLimit * shieldPrecent);
        prevHpLimit = hpLimit;
    }
}

void PlayerState::applyHealValueBuff(sf::Time dt) {
    heal(buff.healValue.apply(0) * dt.asSeconds());
}

// Input
void InputInfo::update() {
    mouseLeftButton = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    mouseRightButton = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
    keyG = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G);
    keyH = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H);
    keyShift = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    keyCtrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
}

// SharedInfo
void SharedInfo::init() {
    playerState.init();
    defencePetalMap = {};
}

bool SharedInfo::update(const sf::RenderWindow& window) {
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
    mouseWorldPosition = window.mapPixelToCoords(mousePixelPos);
    input.update();
    time = timeClock.getElapsedTime();
    dt = dtClock.restart();
    playerState.update();

    if (draggedCard.has_value()) {
        if (draggedCard->update(mouseWorldPosition, dt)) {
            playerState.backpack.add({ draggedCard->getCard(), 1 });
            draggedCard.reset();
            return true;
        }
    }
    return false;
}
