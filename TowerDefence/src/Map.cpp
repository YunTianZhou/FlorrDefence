#include <cassert>
#include "Map.hpp"
#include "AssetManager.hpp"
#include "SpriteCollisionManager.hpp"

static void drawMobDebugBox(sf::RenderTarget& target, sf::RenderStates states, const Mob& mob) {
    // TEST: Draw mob bounds
    sf::RectangleShape globBounds;
    auto b = SpriteCollisionManager::getTrimmedBounds(mob.getSprite());
    sf::Color fill = LIGHT_COLORS.at(mob.getMob().rarity);
    fill.a = 120;
    globBounds.setPosition(b.position);
    globBounds.setSize(b.size);
    globBounds.setFillColor(fill);
    globBounds.setOutlineColor(DARK_COLORS.at(mob.getMob().rarity));
    globBounds.setOutlineThickness(3.f);
    target.draw(globBounds, states);
}

static void drawPetalDebugBox(sf::RenderTarget& target, sf::RenderStates states, const Petal& petal) {
    sf::RectangleShape globBounds;
    auto b = SpriteCollisionManager::getTrimmedBounds(petal.getSprite());
    sf::Color fill = LIGHT_COLORS.at(petal.getCard().rarity);
    fill.a = 200;
    globBounds.setPosition(b.position);
    globBounds.setSize(b.size);
    globBounds.setFillColor(fill);
    globBounds.setOutlineColor(DARK_COLORS.at(petal.getCard().rarity));
    globBounds.setOutlineThickness(3.f);
    target.draw(globBounds, states);
}

// MapInfo
MapInfo::MapInfo(SharedInfo& info)
    : m_info(info) {
}

const Tower* MapInfo::getTower(sf::Vector2i square) const {
    return m_map[square.x][square.y].get();
}

Tower* MapInfo::getTower(sf::Vector2i square) {
    return m_map[square.x][square.y].get();
}

void MapInfo::setCard(sf::Vector2i square, const CardInfo& card) {
    auto tower = Tower::create(m_info, card, square, *this);
    tower->setLength(squareSize.x);
    tower->setOrigin(squareSize / 2.f);
    tower->setPosition(getSquareCenter(square));
    m_map[square.x][square.y] = std::move(tower);
}

void MapInfo::removeCard(sf::Vector2i square) {
    m_map[square.x][square.y].reset();  // Deletes the tower
}

void MapInfo::clear() {
    for (auto& row : m_map)
        for (auto& tower : row)
            tower.reset();  // Clear all towers
}

sf::Vector2i MapInfo::getSquare(sf::Vector2f position) {
    return { int(position.y / squareSize.y)	, int(position.x / squareSize.x) };
}

sf::Vector2f MapInfo::getSquareCenter(sf::Vector2i square) {
    return { square.y * squareSize.y + squareSize.y / 2.0f, square.x * squareSize.x + squareSize.x / 2.0f };
}

// Map
Map::Map(SharedInfo& info)
    : m_info(info), m_map(info), m_spawner(info)
{
    initComponents();
    initSpawner();
}

void Map::update() {
    // Update Buff
    updateBuff();

    // Update Mob Generation
    m_spawner.update(m_info.dt, m_mobs);

    // Update mobs
    for (auto& mob : m_mobs)
        mob->update();

    // Update towers
    for (int row = 0; row < 11; row++) {
        for (int col = 0; col < 10; col++) {
            if (auto tower = m_map.getTower({ row, col })) {
                tower->update();
            }
        }
    }

    // Update petals
    for (auto& petal : m_petals)
        petal->update();

    // Dead entities
    for (auto& dead : m_deadEntities) {
        dead->updateAnimation();
        dead->updatePosition();
    }

    // Tick
    if (m_tickClock.getElapsedTime() >= TICK) {
        tick();
        m_tickClock.restart();
    }
}

void Map::updateBuff() {
    BuffManager& buffManager = m_info.playerState.buffManager;
    for (int row = 0; row < 11; row++) {
        for (int col = 0; col < 10; col++) {
            if (auto tower = m_map.getTower({ row, col }))
                if (auto buffTower = dynamic_cast<BuffTower*>(tower))
                    if (buffTower->getCard().type == "antennae")
                        buffManager.add(buffTower->getCard(), { row, col });
        }
    }

    for (int row = 0; row < 11; row++) {
        for (int col = 0; col < 10; col++) {
            if (auto tower = m_map.getTower({ row, col }))
                if (auto buffTower = dynamic_cast<BuffTower*>(tower))
                    if (buffTower->getCard().type != "antennae")
                        buffManager.add(buffTower->getCard(), { row, col });
        }
    }

    for (auto square : buffManager.get_squares()) {
        if (auto tower = m_map.getTower(square))
            if (auto buffTower = dynamic_cast<BuffTower*>(tower))
                buffTower->applyToBuff();
    }

    m_info.playerState.applyHealBuff(m_info.dt);
}

void Map::tick() {
    tickDeadEntities();

    // Collision Detection (Petal <=> Mob)
    for (auto& petal : m_petals) {
        for (auto& mob : m_mobs) {
            if (mob->isDead()) continue;
            if (SpriteCollisionManager::isCollide(petal->getSprite(), mob->getSprite())) {
                collision(*petal.get(), *mob.get());

                // If petal died after hit, stop checking further
                if (petal->isDead()) break;
            }
        }
    }

    // Mob Tick
    for (auto& mob : m_mobs) {
        if (!mob->isDead())
            mob->tick();
    }

    tickDeadEntities();

    // Towers tick
    for (int row = 0; row < 11; row++) {
        for (int col = 0; col < 10; col++) {
            if (auto tower = m_map.getTower({ row, col })) {
                tower->tick(m_petals, m_mobs);
            }
        }
    }
}

void Map::tickDeadEntities() {
    // Dead Entities
    m_deadEntities.erase(
        std::remove_if(m_deadEntities.begin(), m_deadEntities.end(),
            [&](auto& e) { return e->isDeadAnimationDone(); }),
        m_deadEntities.end()
    );

    // Petals
    for (auto it = m_petals.begin(); it != m_petals.end();) {
        if (it->get()->isDead()) {
            it->get()->onDead();
            m_deadEntities.push_back(std::move(*it));
            it = m_petals.erase(it);
        }
        else {
            ++it;
        }
    }

    // Mobs
    for (auto it = m_mobs.begin(); it != m_mobs.end();) {
        if (it->get()->isDead()) {
            it->get()->onDead();

            for (auto& petal : m_petals) {
                if (auto shoot = dynamic_cast<ShootPetal*>(petal.get())) {
                    shoot->lostTarget(it);
                }
            }

            m_deadEntities.push_back(std::move(*it));
            it = m_mobs.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Map::collision(Petal& petal, Mob& mob) {
    mob.hit(petal.getDamage());
    petal.hit(mob.getDamage());
    petal.applyDebuff(mob.getDebuff());
}

void Map::onEvent(const sf::Event& event) {
    if (!isInside(m_info.mouseWorldPosition))
        return;

    const auto square = m_map.getSquare(m_info.mouseWorldPosition);
    if (auto pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressed->button == sf::Mouse::Button::Left) {
            handlePress(square);
        }
    }
    else if (auto released = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (released->button == sf::Mouse::Button::Left) {
            handleRelease(square);
        }
    }
}

void Map::handlePress(const sf::Vector2i& square) {
    if (m_info.draggedCard.has_value())
        return;

    if (auto tower = m_map.getTower(square)) {
        m_info.draggedCard = DraggedCard(tower->getCard(), square);
        m_map.removeCard(square);
    }
}

void Map::handleRelease(const sf::Vector2i& square) {
    if (!m_info.draggedCard.has_value())
        return;
    if (m_info.draggedCard->isRetreating())
        return;

    auto dragged = std::move(*m_info.draggedCard);
    m_info.draggedCard.reset();

    if (auto target = m_map.getTower(square)) {
        if (auto start = dragged.getStartSquare()) {
            m_map.setCard(*start, target->getCard());
            m_map.setCard(square, dragged.getCard());
        }
        else {
            m_info.draggedCard = DraggedCard(target->getCard(), square);
            m_info.draggedCard->setPosition(m_map.getSquareCenter(square));
            m_info.draggedCard->startRetreat();
            m_map.setCard(square, dragged.getCard());
        }
    }
    else {
        m_map.setCard(square, dragged.getCard());
    }
}

void Map::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    // Background
    target.draw(m_background, states);

    // Towers
    for (int row = 0; row < 11; row++) {
        for (int col = 0; col < 10; col++) {
            if (auto tower = m_map.getTower({ row, col })) {
                target.draw(*tower, states);
            }
        }
    }

    // Petals (Web)
    for (auto& petal : m_petals) {
        if (dynamic_cast<WebPetal*>(petal.get())) {
            if (m_info.input.keyG) {
                drawPetalDebugBox(target, states, *petal);
            }
            target.draw(*petal, states);
        }
    }

    // Mob
    for (const std::string& rarity : RARITIES) {
        for (auto& mob : m_mobs) {
            if (mob->getMob().rarity != rarity)
                continue;

            if (m_info.input.keyH) {
                drawMobDebugBox(target, states, *mob);
            }

            target.draw(*mob, states);
        }
    }

    // Dead Mobs
    for (auto& e : m_deadEntities) {
        if (dynamic_cast<Mob*>(e.get()))
            target.draw(*e, states);
    }

    // Petals
    for (auto& petal : m_petals) {
        if (dynamic_cast<MobPetal*>(petal.get())) {
            if (m_info.input.keyG) {
                drawPetalDebugBox(target, states, *petal);
            }
            target.draw(*petal, states);
        }
    }
    for (auto& petal : m_petals) {
        if (!dynamic_cast<MobPetal*>(petal.get()) &&
            !dynamic_cast<WebPetal*>(petal.get())) {
            if (m_info.input.keyG) {
                drawPetalDebugBox(target, states, *petal);
            }
            target.draw(*petal, states);
        }
    }

    // Dead Petals
    for (auto& e : m_deadEntities) {
        if (dynamic_cast<Petal*>(e.get()))
            target.draw(*e, states);
    }

    // Dragged card
    if (m_info.draggedCard.has_value() && !m_info.draggedCard->isRetreating()) {
        m_highlight.setPosition(m_map.getSquareCenter(m_map.getSquare(m_info.mouseWorldPosition)));
        target.draw(m_highlight, states);
    }
}

bool Map::isInside(sf::Vector2f position) const {
    return bounds.contains(position);
}

void Map::initComponents() {
    // Background
    m_background.setPosition(bounds.position);
    m_background.setSize(bounds.size);
    m_background.setTexture(&AssetManager::getTexture("map"));

    // Highlight
    m_highlight.setSize(m_map.squareSize);
    m_highlight.setOrigin(m_map.squareSize / 2.f);
    m_highlight.setFillColor(sf::Color(255, 255, 255, 100));
}

void Map::initSpawner() {
    m_spawner.addSpawnRecord(Rarity::Common, { "bee",  10.f, 1, 19 });
    m_spawner.addSpawnRecord(Rarity::Unusual, { "bee",   5.f, 5, 39 });
    m_spawner.addSpawnRecord(Rarity::Rare, { "bee",   1.5f, 10, 59 });
    m_spawner.addSpawnRecord(Rarity::Epic, { "bee",   0.6f, 20, 99 });
    m_spawner.addSpawnRecord(Rarity::Legendary, { "bee",  0.1f, 30, 9999 });

    m_spawner.addSpawnRecord(Rarity::Common, { "spider",  9.f, 1, 19 });
    m_spawner.addSpawnRecord(Rarity::Unusual, { "spider",  4.f, 5, 39 });
    m_spawner.addSpawnRecord(Rarity::Rare, { "spider",  1.2f, 15, 59 });
    m_spawner.addSpawnRecord(Rarity::Epic, { "spider",  0.5f, 25, 99 });
    m_spawner.addSpawnRecord(Rarity::Legendary, { "spider", 0.08f, 40, 9999 });

    m_mobs.push_back(std::make_unique<HornetMob>(m_info, MobInfo{ "super", "hornet" }, m_mobs));

}
