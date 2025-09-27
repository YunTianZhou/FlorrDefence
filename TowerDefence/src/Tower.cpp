#include <cmath>
#include <limits>
#include "Tower.hpp"
#include "Map.hpp"

// Tower
std::unique_ptr<Tower> Tower::create(SharedInfo& info, CardInfo card, sf::Vector2i square, const MapInfo& map) {
    std::string type = TOWER_ATTRIBS[card.type].type;
    if (type == "shoot") {
        if (card.type == "triangle")
            return std::make_unique<TriangleTower>(info, card, square, map);
        return std::make_unique<ShootTower>(info, card);
    }
    else if (type == "defence") {
        if (card.type == "web")
            return std::make_unique<WebTower>(info, card, square);
        return std::make_unique<DefenceTower>(info, card, square);
    }
    else if (type == "summon") {
        return std::make_unique<SummonTower>(info, card);
    }
    else if (type == "buff") {
        if (card.type == "rose")
            return std::make_unique<RoseTower>(info, card, square);
        return std::make_unique<BuffTower>(info, card, square);
    }
    else if (type == "multi_shoot") {
        return std::make_unique<MultiShotTower>(info, card);
    }
    else {
        return std::make_unique<Tower>(info, card);
    }
}

Tower::Tower(SharedInfo& info, CardInfo card)
	: m_info(info), m_attribs(TOWER_ATTRIBS[card.type][card.rarity]) {
	m_card.setCard(card);

    m_info.counter.tower[getCard()]++;
}

Tower::~Tower() {
    m_info.counter.tower[getCard()]--;
}

void Tower::update() {}

void Tower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {}

void Tower::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();

	target.draw(m_card, states);
}

// ShootTower
ShootTower::ShootTower(SharedInfo& info, CardInfo card)
	: Tower(info, card) {

}

void ShootTower::update() {
    float elapsedTime = m_reloadClock.getElapsedTime().asSeconds();
    m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
}

void ShootTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
	if (m_reloadClock.getElapsedTime().asSeconds() > getBuffedAttrib("reload")) {
        std::optional nearestMob = getNearestMob(mobs);
        if (nearestMob) {
            petals.push_back(ShootPetal::create(m_info, m_card.getCard(), getPosition(), *nearestMob));
            m_reloadClock.restart();
        }
	}
}

std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> ShootTower::getNearestMob(const std::list<std::unique_ptr<Mob>>& mobs) const {
    float range = getAttrib("range") * MapInfo::squareSize.x;
    float rangeSquared = range * range;
    auto towerPos = getPosition();

    std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> nearestMob;
    float nearestDistSquared = std::numeric_limits<float>::max();

    for (auto it = mobs.begin(); it != mobs.end(); ++it) {
        auto mobPos = it->get()->getPosition();
        float dx = mobPos.x - towerPos.x;
        float dy = mobPos.y - towerPos.y;
        float distSquared = dx * dx + dy * dy;

        if (distSquared <= rangeSquared && distSquared < nearestDistSquared) {
            nearestDistSquared = distSquared;
            nearestMob = it;
        }
    }

    return nearestMob;
}

// DefenceTower
DefenceTower::DefenceTower(SharedInfo& info, CardInfo card, sf::Vector2i square)
    : m_square(square), Tower(info, card) {}

void DefenceTower::update() {
    if (auto defence = m_info.defencePetalMap[m_square.x][m_square.y]) {
        m_reloadClock.restart();
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
        float elapsedTime = m_reloadClock.getElapsedTime().asSeconds();
        m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
    }
}

void DefenceTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadClock.getElapsedTime().asSeconds() > getBuffedAttrib("reload")) {
        if (!m_info.defencePetalMap[m_square.x][m_square.y]) {
            petals.push_back(DefencePetal::create(m_info, m_card.getCard(), m_square));
            m_reloadClock.restart();
        }
    }
}

// SummonTower
SummonTower::SummonTower(SharedInfo& info, CardInfo card) 
    : Tower(info, card) {}

void SummonTower::update() {
    if (!ableToSummon()) {
        m_reloadClock.stop();
        m_card.setReload(0.f, true);
    }
    else {
        m_reloadClock.start();
        float elapsedTime = m_reloadClock.getElapsedTime().asSeconds();
        m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
    }
}

void SummonTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadClock.getElapsedTime().asSeconds() > getBuffedAttrib("reload")) {
        if (ableToSummon()) {
            petals.push_back(MobPetal::create(m_info, getCard()));
            m_reloadClock.restart();
        }
    }
}

bool SummonTower::ableToSummon() {
    return m_info.counter.petal[getCard()] < m_info.counter.tower[getCard()] * getAttrib("copy");
}

// Buff Tower
BuffTower::BuffTower(SharedInfo& info, CardInfo card, sf::Vector2i square)
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
        if (name.find("_buff") != std::string::npos)
            m_info.playerState.buff.add(name, value);
    }
}

bool BuffTower::isActive() const {
    return m_info.playerState.buffManager.find(getCard(), m_square);
}

// MultiShotTower (Shoot Tower)
MultiShotTower::MultiShotTower(SharedInfo& info, CardInfo card)
    : ShootTower(info, card) {
}

std::vector<std::list<std::unique_ptr<Mob>>::const_iterator>
MultiShotTower::getTargets(const std::list<std::unique_ptr<Mob>>& mobs) const {
    const float range = getAttrib("range") * MapInfo::squareSize.x;
    const float rangeSq = range * range;
    const int maxTargets = static_cast<int>(getAttrib("copy"));
    auto towerPos = getPosition();

    std::vector<std::pair<
        std::list<std::unique_ptr<Mob>>::const_iterator,
        float>> best;

    for (auto it = mobs.begin(); it != mobs.end(); ++it) {
        auto mp = (*it)->getPosition();
        float dx = mp.x - towerPos.x;
        float dy = mp.y - towerPos.y;
        float d2 = dx * dx + dy * dy;
        if (d2 > rangeSq) continue;

        if ((int)best.size() < maxTargets) {
            best.emplace_back(it, d2);
            std::sort(best.begin(), best.end(),
                [](auto& a, auto& b) { return a.second < b.second; });
        }
        else if (d2 < best.back().second) {
            best.back() = { it, d2 };
            for (int i = (int)best.size() - 1; i > 0; --i) {
                if (best[i].second < best[i - 1].second)
                    std::swap(best[i], best[i - 1]);
                else
                    break;
            }
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
    if (m_reloadClock.getElapsedTime().asSeconds() > getBuffedAttrib("reload")) {
        auto targets = getTargets(mobs);
        if (!targets.empty()) {
            for (auto it : targets) {
                petals.push_back(
                    ShootPetal::create(m_info, m_card.getCard(),
                        getPosition(), it));
            }
            m_reloadClock.restart();
        }
    }
}

// Web (Defence Tower)
WebTower::WebTower(SharedInfo& info, CardInfo card, sf::Vector2i square)
    : DefenceTower(info, card, square) {
}

void WebTower::update() {
    if (auto defence = m_info.defencePetalMap[m_square.x][m_square.y]) {
        m_reloadClock.restart();
        if (defence->getCard() == getCard()) {
            auto web = dynamic_cast<const WebPetal*>(defence);
            m_card.setReload(web->getDelta(), true);
        }
        else {
            m_card.setReload(0.f, true);
        }
    }
    else {
        float elapsedTime = m_reloadClock.getElapsedTime().asSeconds();
        m_card.setReload(std::min(1.0f, (elapsedTime / getBuffedAttrib("reload"))), false);
    }
}

// Rose Tower (Buff Tower)
RoseTower::RoseTower(SharedInfo& info, CardInfo card, sf::Vector2i square)
    : BuffTower(info, card, square) {
}

void RoseTower::update() {
    if (isActive()) {
        float elapseTime = m_reloadClock.getElapsedTime().asSeconds();
        m_card.setReload(std::min(1.0f, (elapseTime / getBuffedAttrib("reload"))), false);
    }
    else {
        m_card.setReload(0.f, true);
        m_reloadClock.restart();
    }
}

void RoseTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadClock.getElapsedTime().asSeconds() > getBuffedAttrib("reload")) {
        m_info.playerState.heal(getAttrib("heal"));
        m_reloadClock.restart();
    }
}

// Triangle Tower (Shoot Tower)
TriangleTower::TriangleTower(SharedInfo& info, CardInfo card, sf::Vector2i square, const MapInfo& map)
    : ShootTower(info, card), m_square(square), m_map(map) {}

void TriangleTower::tick(std::list<std::unique_ptr<Petal>>& petals, const std::list<std::unique_ptr<Mob>>& mobs) {
    if (m_reloadClock.getElapsedTime().asSeconds() > getBuffedAttrib("reload")) {
        std::optional nearestMob = getNearestMob(mobs);
        if (nearestMob) {
            petals.push_back(std::make_unique<TrianglePetal>(m_info, m_card.getCard(), getPosition(), *nearestMob, countAdjacentSameType()));
            m_reloadClock.restart();
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

