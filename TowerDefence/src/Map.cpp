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
using ST = MapInfo::SquareType;

std::array<std::array<ST, 10>, 11> MapInfo::m_squareTypeMap = { {
    { ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Slot, ST::Grass, ST::Obstacle, ST::Grass, ST::Grass },
    { ST::Obstacle, ST::Grass, ST::Trail, ST::Trail, ST::Trail, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass },
    { ST::Grass, ST::Grass, ST::Trail, ST::Grass, ST::Trail, ST::Grass, ST::Trail, ST::Trail, ST::Trail, ST::Trail },
    { ST::Grass, ST::Grass, ST::Trail, ST::Grass, ST::Trail, ST::Grass, ST::Trail, ST::Grass, ST::Slot, ST::Grass },
    { ST::Grass, ST::Grass, ST::Trail, ST::Grass, ST::Trail, ST::Grass, ST::Trail, ST::Grass, ST::Grass, ST::Obstacle },
    { ST::Trail, ST::Trail, ST::Trail, ST::Grass, ST::Trail, ST::Grass, ST::Trail, ST::Grass, ST::Grass, ST::Grass },
    { ST::Grass, ST::Grass, ST::Grass, ST::Slot, ST::Trail, ST::Grass, ST::Trail, ST::Trail, ST::Trail, ST::Grass },
    { ST::Obstacle, ST::Trail, ST::Trail, ST::Trail, ST::Trail, ST::Grass, ST::Grass, ST::Grass, ST::Trail, ST::Obstacle },
    { ST::Grass, ST::Trail, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Slot, ST::Trail, ST::Grass },
    { ST::Grass, ST::Trail, ST::Trail, ST::Trail, ST::Trail, ST::Trail, ST::Trail, ST::Trail, ST::Trail, ST::Grass },
    { ST::Obstacle, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass, ST::Grass },
} };

MapInfo::MapInfo(SharedInfo& info, Map& map)
    : m_info(info), m_mapRef(map) {
}

const Tower* MapInfo::getTower(sf::Vector2i square) const {
    return m_map[square.x][square.y].get();
}

Tower* MapInfo::getTower(sf::Vector2i square) {
    return m_map[square.x][square.y].get();
}

void MapInfo::setCard(sf::Vector2i square, const CardInfo& card) {
    if (!isPlaceable(square, card))
        return;

    if (!isEmpty(square))
        removeCard(square);

    auto tower = Tower::create(m_info, card, square, m_mapRef);
    tower->setLength(squareSize.x);
    tower->setOrigin(squareSize / 2.f);
    tower->setPosition(getSquareCenter(square));
    m_map[square.x][square.y] = std::move(tower);

    m_buffUpdated = false;
}

void MapInfo::removeCard(sf::Vector2i square) {
    if (isEmpty(square))
        return;

    m_map[square.x][square.y].reset();  // Deletes the tower

    m_buffUpdated = false;
}

int MapInfo::removeAll(const CardInfo& card) {
    int count = 0;

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 10; j++) {
            sf::Vector2i square = { i, j };
            if (!isEmpty(square) && getTower(square)->getCard() == card) {
                removeCard(square);
                count++;
            }
        }
    }

    return count;
}

void MapInfo::clear() {
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 10; j++) {
            removeCard({ i, j });
        }
    }
}

void MapInfo::updateTowerBuff() {
    PlayerState& player = m_info.playerState;
    player.towerBuff.reset();

    int anteaneaLevel = (int)player.talentBuff.antennae.apply(0);
    for (int row = 0; row < 11; row++) {
        for (int col = 0; col < 10; col++) {
            if (auto tower = getTower({ row, col }))
                if (tower->getCard().type == "antennae")
                    anteaneaLevel = std::max(anteaneaLevel,
                        RARITIE_LEVELS.at(tower->getCard().rarity));
        }
    }

    BuffManager& buffManager = m_info.playerState.buffManager;
    buffManager.setAntennaeLevel(anteaneaLevel);

    for (int row = 0; row < 11; row++) {
        for (int col = 0; col < 10; col++) {
            if (auto tower = getTower({ row, col }))
                if (auto buffTower = dynamic_cast<BuffTower*>(tower))
                    buffManager.add(buffTower->getCard(), { row, col });
        }
    }

    for (auto square : buffManager.getSquares()) {
        if (auto tower = getTower(square))
            if (auto buffTower = dynamic_cast<BuffTower*>(tower))
                buffTower->applyToBuff();
    }

    m_buffUpdated = true;
}

void MapInfo::update() {
    if (!m_buffUpdated)
        updateTowerBuff();
    m_info.playerState.applyHealValueBuff(m_info.dt);
}

void MapInfo::tick() {
    updateTowerBuff();
    m_info.playerState.buff.mergeFrom(m_info.playerState.towerBuff, m_info.playerState.talentBuff);
}

bool MapInfo::isValid(sf::Vector2i square) const {
    return square.x >= 0 && square.x < 11 && square.y >= 0 && square.y < 10;
}

bool MapInfo::isEmpty(sf::Vector2i square) const {
    return m_map[square.x][square.y] == nullptr;
}

bool MapInfo::isPlaceable(sf::Vector2i square, const CardInfo& card) const {
    ST squareType = m_squareTypeMap[square.x][square.y];

    if (squareType == ST::Obstacle)
        return false;
    
    std::string towerType = TOWER_ATTRIBS.at(card.type).type;
    if (towerType == "defence")
        return squareType == ST::Trail;
    else if (towerType == "buff")
        return squareType == ST::Slot;
    else
        return squareType == ST::Grass;
}

bool MapInfo::findSquareAndPlace(const CardInfo& card) {
    std::string towerType = TOWER_ATTRIBS.at(card.type).type;

    if (towerType == "defence") {
        for (sf::Vector2i square : PATH_SQUARES) {
            if (isEmpty(square) && isPlaceable(square, card)) {
                // Found a place to place that tower
                setCard(square, card);
                return true;
			}
        }
    }
    else {
        for (int i = 0; i < 11; i++) {
            for (int j = 0; j < 10; j++) {
                sf::Vector2i square = { i, j };
                if (isEmpty(square) && isPlaceable(square, card)) {
                    // Found a place to place that tower
                    setCard(square, card);
                    return true;
                }
            }
        }
    }
    
    // Unable to find a square to place that tower
    return false;
}

sf::Vector2i MapInfo::getSquare(sf::Vector2f position) {
    return { int(position.y / squareSize.y)	, int(position.x / squareSize.x) };
}

sf::Vector2f MapInfo::getSquareCenter(sf::Vector2i square) {
    return { square.y * squareSize.y + squareSize.y / 2.0f, square.x * squareSize.x + squareSize.x / 2.0f };
}

// Map
Map::Map(SharedInfo& info)
    : m_info(info), m_map(info, *this), m_spawner(info)
{
    initComponents();
}

bool Map::update() {
    // Sub Map
    m_map.update();

    // Update Mob Generation
    m_spawner.update(m_mobs);

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

    // Effects
    for (auto& effect : m_effects)
        effect->update();

    // Tick
    if (m_tickClock.getElapsedTime() >= TICK) {
        tick();
        m_tickClock.restart();
    }

    // Put card request
    return handlePlaceTowerRequest();
}

void Map::tick() {
    // Sub Map
    m_map.tick();

    // Dead Entities
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

    // Tower tick
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

    // Effects
    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
            [&](auto& e) { return e->isDone(); }),
        m_effects.end()
    );
}

void Map::collision(Petal& petal, Mob& mob) {
    if (petal.getCard().type == "web" && mob.getMob().type == "spider")
        return;  // Web doesn't effect spider
    if (petal.getCard().type == "lightning") {
        LightningPetal& lightning = dynamic_cast<LightningPetal&>(petal);
        lightning.onHit(mob, m_mobs, m_effects);
    }

    mob.hit(petal.getDamage(), petal.getDamageType());
    petal.hit(mob.getDamage());
    petal.applyDebuff(mob.getDebuff());
}

bool Map::onEvent(const sf::Event& event) {
    if (!isInside(m_info.mouseWorldPosition))
        return false;

    const auto square = m_map.getSquare(m_info.mouseWorldPosition);
    if (auto pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressed->button == sf::Mouse::Button::Left) {
            handlePress(square);
        }
        else if (pressed->button == sf::Mouse::Button::Right) {
            return handleRightPress(square);;
        }
    }
    else if (auto released = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (released->button == sf::Mouse::Button::Left) {
            handleRelease(square);
        }
    }

    return false;
}

void Map::handlePress(const sf::Vector2i& square) {
    if (m_info.draggedCard.has_value())
        return;

    if (auto tower = m_map.getTower(square)) {
        m_info.draggedCard = DraggedCard(tower->getCard(), square);
        m_map.removeCard(square);
    }
}

bool Map::handleRightPress(const sf::Vector2i& square) {
    if (m_info.draggedCard.has_value())
        return false;

    if (auto* tower = m_map.getTower(square)) {
        CardInfo card = tower->getCard();
        if (m_info.input.keyShift) {
            int count = m_map.removeAll(card);
            m_info.playerState.backpack.add({ card, count });
        }
        else {
            m_map.removeCard(square);
            m_info.playerState.backpack.add({ card, 1 });
        }

        return true;
    }

    return false;
}

void Map::handleRelease(const sf::Vector2i& square) {
    if (!m_info.draggedCard.has_value())
        return;
    if (m_info.draggedCard->isRetreating())
        return;

    auto dragged = std::move(*m_info.draggedCard);
    m_info.draggedCard.reset();

    if (m_map.isPlaceable(square, dragged.getCard())) {
        if (auto* target = m_map.getTower(square)) {
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
    else {
        if (auto start = dragged.getStartSquare()) {
            m_map.setCard(*start, dragged.getCard());
        }
        else {
            m_info.draggedCard = DraggedCard(dragged.getCard(), square);
            m_info.draggedCard->setPosition(m_map.getSquareCenter(square));
            m_info.draggedCard->startRetreat();
        }
    }
}

bool Map::handlePlaceTowerRequest() {
    if (!m_info.placeRequest.has_value())
        return false;

    CardStackInfo stack = m_info.placeRequest.value();
    m_info.placeRequest.reset();

    stack.count = std::min(stack.count, m_info.playerState.backpack.getCount(stack.card));
    while (stack.count > 0 && m_map.findSquareAndPlace(stack.card)) {
        stack.count--;
        m_info.playerState.backpack.add({ stack.card, -1 });
    }

    return false;
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

    // Effects
    for (auto& effect : m_effects)
        target.draw(*effect, states);

    // Dragged card
    if (m_info.draggedCard.has_value() && !m_info.draggedCard->isRetreating()) {
        sf::Vector2i mouseSquare = m_map.getSquare(m_info.mouseWorldPosition);

        if (m_map.isValid(mouseSquare)) {
            m_highlight.setPosition(m_map.getSquareCenter(mouseSquare));
            if (m_map.isPlaceable(mouseSquare, m_info.draggedCard->getCard()))
                m_highlight.setFillColor(sf::Color(255, 255, 255, 100));
            else
                m_highlight.setFillColor(sf::Color(255, 100, 100, 120));

            target.draw(m_highlight, states);
        }
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
}
