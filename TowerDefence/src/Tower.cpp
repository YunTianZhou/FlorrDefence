#include <cmath>
#include <limits>
#include "Tower.hpp"
#include "Map.hpp"
#include "Tools.hpp"

// Tower
std::unique_ptr<Tower> Tower::create(SharedInfo& info, const CardInfo& card, sf::Vector2i square, Map& map) {
    std::string type = TOWER_ATTRIBS[card.type].type;
    if (type == "shoot") {
        if (card.type == "triangle")
            return std::make_unique<TriangleTower>(info, card, square, map.getMapInfo());
        else if (card.type == "laser")
            return std::make_unique<LaserTower>(info, card, square, map);
        return std::make_unique<ShootTower>(info, card);
    }
    else if (type == "defence") {
        if (card.type == "web")
            return std::make_unique<WebTower>(info, card, square);
        else if (card.type == "pollen")
            return std::make_unique<PollenTower>(info, card, square);
        else if (card.type == "shovel")
            return std::make_unique<ShovelTower>(info, card, square);
        return std::make_unique<DefenceTower>(info, card, square);
    }
    else if (type == "summon") {
        return std::make_unique<SummonTower>(info, card);
    }
    else if (type == "buff") {
        if (card.type == "rose")
            return std::make_unique<RoseTower>(info, card, square);
        else if (card.type == "coin")
            return std::make_unique<CoinTower>(info, card, square);
        return std::make_unique<BuffTower>(info, card, square);
    }
    else if (type == "multi_shoot") {
        return std::make_unique<MultiShotTower>(info, card);
    }
    else {
        return std::make_unique<Tower>(info, card);
    }
}

Tower::Tower(SharedInfo& info, const CardInfo& card)
	: m_info(info), m_attribs(TOWER_ATTRIBS[card.type][card.rarity]) {
	m_card.setCard(card);

    m_info.counter.tower[getCard()]++;
}

Tower::~Tower() {
    m_info.counter.tower[getCard()]--;
}

void Tower::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();

	target.draw(m_card, states);
}

// ShootTower
ShootTower::ShootTower(SharedInfo& info, const CardInfo& card)
	: Tower(info, card) {

}

void ShootTower::update() {
    m_reloadTimer += m_info.dt;
    float elapsedTime = m_reloadTimer.asSeconds();
    m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
}

void ShootTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
	if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        std::optional nearestMob = getNearestMob(mobs);
        if (nearestMob) {
            petals.push_back(ShootPetal::create(m_info, m_card.getCard(), getPosition(), *nearestMob));
            m_reloadTimer = sf::Time::Zero;
        }
	}
}

std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> ShootTower::getNearestMob(const std::list<std::unique_ptr<Mob>>& mobs) const {
    float range = m_info.playerState.buff.reach.apply(getAttrib("range") * MapInfo::squareSize.x);
    float rangeSquared = range * range;
    auto towerPos = getPosition();

    std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> nearestMob;
    float nearestDistSquared = std::numeric_limits<float>::max();

    for (auto it = mobs.begin(); it != mobs.end(); it++) {
        float distSquared = getDistanceSquare(towerPos, it->get()->getPosition());

        if (distSquared <= rangeSquared && distSquared < nearestDistSquared) {
            nearestDistSquared = distSquared;
            nearestMob = it;
        }
    }

    return nearestMob;
}

// DefenceTower
DefenceTower::DefenceTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square)
    : m_square(square), Tower(info, card) {}

void DefenceTower::update() {
    if (auto defence = m_info.defencePetalMap[m_square.x][m_square.y]) {
        m_reloadTimer = sf::Time::Zero;
        if (defence->getCard() == getCard()) {
            int hp = defence->getHp();
            int totalHp = int(getAttrib("hp"));
            m_card.setReload(float(hp) / totalHp, true);
        }
        else {
            m_card.setReload(0.f, true);
        }
    }
    else {
        m_reloadTimer += m_info.dt;
        float elapsedTime = m_reloadTimer.asSeconds();
        m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
    }
}

void DefenceTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        if (!m_info.defencePetalMap[m_square.x][m_square.y]) {
            petals.push_back(DefencePetal::create(m_info, m_card.getCard(), m_square));
            m_reloadTimer = sf::Time::Zero;
        }
    }
}

// SummonTower
SummonTower::SummonTower(SharedInfo& info, const CardInfo& card) 
    : Tower(info, card) {}

void SummonTower::update() {
    if (!ableToSummon()) {
        m_reloadTimer = sf::Time::Zero;
        m_card.setReload(0.f, true);
    }
    else {
        m_reloadTimer += m_info.dt;
        float elapsedTime = m_reloadTimer.asSeconds();
        m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
    }
}

void SummonTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        if (ableToSummon()) {
            petals.push_back(MobPetal::create(m_info, getCard()));
            m_reloadTimer = sf::Time::Zero;
        }
    }
}

bool SummonTower::ableToSummon() {
    return m_info.counter.petal[getCard()] < m_info.counter.tower[getCard()] * getAttrib("copy");
}

// Buff Tower
BuffTower::BuffTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square)
    : m_square(square), Tower(info, card) {

}

void BuffTower::update() {
    if (isActive())
        m_card.setReload(1.f, true);
    else
        m_card.setReload(0.f, true);
}

void BuffTower::applyToBuff() {
    for (auto [name, value] : m_attribs.attribs) {
        if (name.find("_buff") != std::string::npos) {
            std::string key = name.substr(0, name.size() - 5);
            m_info.playerState.towerBuff.get(key).add(value);
        }
    }
}

bool BuffTower::isActive() const {
    return m_info.playerState.buffManager.find(getCard(), m_square);
}

// MultiShotTower (Shoot Tower)
MultiShotTower::MultiShotTower(SharedInfo& info, const CardInfo& card)
    : ShootTower(info, card) {
}

std::vector<std::list<std::unique_ptr<Mob>>::const_iterator>
MultiShotTower::getTargets(const std::list<std::unique_ptr<Mob>>& mobs) const {
    const float range = m_info.playerState.buff.reach.apply(getAttrib("range") * MapInfo::squareSize.x);
    const float rangeSq = range * range;
    const int maxTargets = (int)(getAttrib("copy"));
    auto towerPos = getPosition();

    std::vector<std::pair<
        std::list<std::unique_ptr<Mob>>::const_iterator,
        float>> best;

    for (auto it = mobs.begin(); it != mobs.end(); it++) {
        float d2 = getDistanceSquare(towerPos, it->get()->getPosition());
        if (d2 > rangeSq) continue;

        if (best.size() < maxTargets) {
            best.emplace_back(it, d2);
            int i = (int)best.size() - 1;
            while (i > 0 && best[i].second < best[i - 1].second) {
                std::swap(best[i], best[i - 1]);
                i--;
            }
        }
        else if (d2 < best.back().second) {
            int i = (int)best.size() - 1;
            while (i > 0 && best[i - 1].second > d2) {
                best[i] = best[i - 1];
                i--;
            }
            best[i] = std::pair{ it, d2 };
        }
    }

    std::vector<std::list<std::unique_ptr<Mob>>::const_iterator> targets;
    targets.reserve(best.size());
    for (auto& p : best)
        targets.push_back(p.first);

    return targets;
}

void MultiShotTower::tick(std::list<std::unique_ptr<Petal>>& petals,
    const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        auto targets = getTargets(mobs);
        if (!targets.empty()) {
            for (auto it : targets) {
                petals.push_back(
                    ShootPetal::create(m_info, m_card.getCard(),
                        getPosition(), it));
            }
            m_reloadTimer = sf::Time::Zero;
        }
    }
}

// Web (Defence Tower)
WebTower::WebTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square)
    : DefenceTower(info, card, square) {
}

void WebTower::update() {
    if (auto defence = m_info.defencePetalMap[m_square.x][m_square.y]) {
        m_reloadTimer = sf::Time::Zero;
        if (defence->getCard() == getCard()) {
            auto web = dynamic_cast<const WebPetal*>(defence);
            m_card.setReload(web->getDelta(), true);
        }
        else {
            m_card.setReload(0.f, true);
        }
    }
    else {
        m_reloadTimer += m_info.dt;
        float elapsedTime = m_reloadTimer.asSeconds();
        m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
    }
}

// Pollen Tower (Shoot Tower)
void PollenTower::update() {
    m_reloadTimer += m_info.dt;
    float elapsedTime = m_reloadTimer.asSeconds();
    m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
}

void PollenTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() < getBuffedAttrib("reload"))
        return;
    
    int index = 0;
    while (index < PATH_SQUARES.size() && PATH_SQUARES[index] != m_square)
        index++;
    assert(index < PATH_SQUARES.size());

    int left = index;
    int right = index;
    int copyLeft = (int)getAttrib("copy");
    while (copyLeft > 0 && (left > 0 || right < PATH_SQUARES.size())) {
        if (left >= 0) {
            sf::Vector2i leftSq = PATH_SQUARES[left];
            if (m_info.defencePetalMap[leftSq.x][leftSq.y] == nullptr)
                petals.push_back(DefencePetal::create(m_info, m_card.getCard(), leftSq));
            copyLeft--;
        }
        left--;

        if (right != index && copyLeft > 0 && right < PATH_SQUARES.size()) {
            sf::Vector2i rightSq = PATH_SQUARES[right];
            if (m_info.defencePetalMap[rightSq.x][rightSq.y] == nullptr)
                petals.push_back(DefencePetal::create(m_info, m_card.getCard(), rightSq));
            copyLeft--;
        }
        right++;
    }

    m_reloadTimer = sf::Time::Zero;
}

// Shovel Tower (Defence Tower)
void ShovelTower::update() {
    if (auto defence = m_info.defencePetalMap[m_square.x][m_square.y]) {
        int rarity = RARITIE_LEVELS.at(m_card.getCard().rarity);
        int petalRarity = RARITIE_LEVELS.at(defence->getCard().rarity);
        if (rarity >= petalRarity) {
            m_reloadTimer += m_info.dt;
            float elapsedTime = m_reloadTimer.asSeconds();
            m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
            return;
        }
    }

    m_reloadTimer = sf::Time::Zero;
    m_card.setReload(0.f, true);
}

void ShovelTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        if (auto defence = m_info.defencePetalMap[m_square.x][m_square.y]) {
            int rarity = RARITIE_LEVELS.at(m_card.getCard().rarity);
            int petalRarity = RARITIE_LEVELS.at(defence->getCard().rarity);
            if (rarity >= petalRarity) {
                defence->kill();
                int64_t coin = TOWER_ATTRIBS["shovel"].rarities[defence->getCard().rarity].coin;
                m_info.playerState.addCoin(coin);
            }
        }
        m_reloadTimer = sf::Time::Zero;
    }
}

// Rose Tower (Buff Tower)
RoseTower::RoseTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square)
    : BuffTower(info, card, square) {
}

void RoseTower::update() {
    if (isActive()) {
        m_reloadTimer += m_info.dt;
        float elapseTime = m_reloadTimer.asSeconds();
        m_card.setReload(std::min(1.0f, (elapseTime / getBuffedAttrib("reload"))), false);
    }
    else {
        m_card.setReload(0.f, true);
        m_reloadTimer = sf::Time::Zero;
    }
}

void RoseTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        m_info.playerState.heal(getAttrib("heal"));
        m_reloadTimer = sf::Time::Zero;
    }
}

// Coin Tower (Buff Tower)
CoinTower::CoinTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square)
    : BuffTower(info, card, square) {
}

void CoinTower::update() {
    if (isActive()) {
        m_reloadTimer += m_info.dt;
        float elapseTime = m_reloadTimer.asSeconds();
        m_card.setReload(std::min(1.0f, (elapseTime / getBuffedAttrib("reload"))), false);
    }
    else {
        m_card.setReload(0.f, true);
        m_reloadTimer = sf::Time::Zero;
    }
}

void CoinTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        m_info.playerState.addCoin(m_attribs.coin);
        m_reloadTimer = sf::Time::Zero;
    }
}

// Triangle Tower (Shoot Tower)
TriangleTower::TriangleTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square, const MapInfo& map)
    : ShootTower(info, card), m_square(square), m_map(map) {}

void TriangleTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadTimer.asSeconds() > getBuffedAttrib("reload")) {
        std::optional nearestMob = getNearestMob(mobs);
        if (nearestMob) {
            petals.push_back(std::make_unique<TrianglePetal>(m_info, m_card.getCard(), getPosition(), *nearestMob, countAdjacentSameType()));
            m_reloadTimer = sf::Time::Zero;
        }
    }
}

int TriangleTower::countAdjacentSameType() {
    static std::vector<int> dx = { 1, -1, 0, 0, 1, -1, -1, 1 };
    static std::vector<int> dy = { 0, 0, 1, -1, 1, -1, 1, -1 };

    int count = 0;
    for (int d = 0; d < 8; d++) {
        sf::Vector2i adj_square = m_square + sf::Vector2i(dx[d], dy[d]);
        if (adj_square.x >= 0 && adj_square.x < 11 && adj_square.y >= 0 && adj_square.y < 10) {
            const Tower* adj_tower = m_map.getTower(adj_square);
            if (adj_tower && adj_tower->getCard() == getCard()) {
                count++;
            }
        }
    }

    return count;
}

// Laser Tower (Shoot Tower)
LaserTower::LaserTower(SharedInfo& info, const CardInfo& card, sf::Vector2i square, Map& map)
    : ShootTower(info, card), m_square(square), m_map(map) {
    m_card.setReload(1.f, false);
}

void LaserTower::update() {
    if (!m_info.laserMap[m_square.x][m_square.y])
        m_map.getPetals().push_back(std::make_unique<LaserPetal>(m_info, m_card.getCard(), m_square, m_map.getMapInfo(), m_map.getMobs()));
}
