#include "Petal.hpp"
#include "Map.hpp"
#include "AssetManager.hpp"

// Petal
Petal::Petal(SharedInfo& info, CardInfo card)
	: m_attribs(TOWER_ATTRIBS[card.type][card.rarity]), m_card(card),
	Entity(info, AssetManager::getPetalTexture(card.type)) {
	setScale(0.32f);
	setFlash(sf::Color(180, 0, 0), 0.4f);
	m_hp = (int)getAttrib("hp");

	m_info.counter.petal[card]++;
}

Petal::Petal(SharedInfo& info, CardInfo card, const sf::Texture& texture)
	: m_attribs(TOWER_ATTRIBS[card.type][card.rarity]), m_card(card),
	Entity(info, texture) {
	setFlash(sf::Color(180, 0, 0), 0.4f);

	m_info.counter.petal[card]++;
}

int Petal::getArmor() const {
	return hasAttrib("armor") ? int(getAttrib("armor")) : 0;
}

int Petal::getDamage() const {
	return int(getAttrib("damage"));
}

void Petal::onDead() {
	if (hasAttrib("death_heal"))
		m_info.playerState.heal(getAttrib("death_heal"));

	m_info.counter.petal[getCard()]--;
}

// ShootPetal
std::unique_ptr<ShootPetal> ShootPetal::create(SharedInfo& info, CardInfo card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target) {
	return std::make_unique<ShootPetal>(info, card, startPosition, target);
}

const std::map<std::string, sf::Angle> ShootPetal::petalTilt = {
	{"basic", sf::degrees(0.f)},
	{"bone", sf::degrees(-32.f)},
	{"corn", sf::degrees(40.f)},
	{"missile", sf::degrees(148.f)},
	{"light", sf::degrees(0.f)},
	{"stinger", sf::degrees(90.f)},
	{"pollen", sf::degrees(0.f)},
	{"triangle", sf::degrees(270.f)}
};

ShootPetal::ShootPetal(SharedInfo& info, CardInfo card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target)
	: Petal(info, card), m_startPosition(startPosition), m_target(target) {
	m_sprite.setPosition(startPosition);

	// Face target
	sf::Vector2f mobPos = m_target.value()->get()->getPosition();
	sf::Vector2f delta = mobPos - getPosition();
	if (delta.x != 0.f || delta.y != 0.f) {
		float angleRad = std::atan2(delta.y, delta.x);
		m_direction = sf::radians(angleRad);
		m_sprite.setRotation(m_direction + sf::degrees(90.f) - petalTilt.at(getCard().type));
	}
}

void ShootPetal::update() {
	// Animation
	updateAnimation();

	// Move
	updatePosition();

	// Check if out of bounds
	sf::Vector2f newPosition = getPosition();
	if (!Map::bounds.contains(newPosition)) {
		kill();
		return;
	}

	sf::Vector2f delta = newPosition - m_startPosition;
	float dstSquare = delta.x * delta.x + delta.y * delta.y;
	float range = getAttrib("range") * MapInfo::squareSize.x;
	if (dstSquare > range * range) {
		kill();
		return;
	}
}

void ShootPetal::updatePosition() {
	// Direction
	updateDirection();

	// Move
	float speed = getBuffedAttrib("speed") * MapInfo::squareSize.x;
	sf::Vector2f offset(
		std::cos(m_direction.asRadians()),
		std::sin(m_direction.asRadians())
	);
	m_sprite.move(offset * speed * m_info.dt.asSeconds());
}

void ShootPetal::lostTarget() {
	m_target.reset();
}

void ShootPetal::lostTarget(std::list<std::unique_ptr<Mob>>::const_iterator target) {
	if (m_target == target)
		lostTarget();
}

void ShootPetal::onDead() {
	Petal::onDead();

	lostTarget();
}

void ShootPetal::updateDirection() {
	if (!m_target.has_value())
		return;

	sf::Vector2f mobPos = m_target.value()->get()->getPosition();
	sf::Vector2f delta = mobPos - getPosition();

	if (std::abs(delta.x) <= 3.f && std::abs(delta.y) <= 3.f)
		return;

	float desired = std::atan2(delta.y, delta.x);
	float current = m_direction.asRadians();

	constexpr float PI = 3.1415927f;
	float diff = desired - current;
	while (diff <= -PI) diff += 2.f * PI;
	while (diff > PI) diff -= 2.f * PI;

	float turnDeg = 540.f;
	float turnRadPerSec = turnDeg * (PI / 180.f);
	float maxStep = turnRadPerSec * m_info.dt.asSeconds();

	float step = std::max(-maxStep, std::min(maxStep, diff));
	float next = current + step;
	m_direction = sf::radians(next);

	m_sprite.setRotation(m_direction + sf::degrees(90.f) - petalTilt.at(getCard().type));
}

// DefencePetal
std::unique_ptr<DefencePetal> DefencePetal::create(SharedInfo& info, CardInfo card, sf::Vector2i square) {
	if (card.type == "web")
		return std::make_unique<WebPetal>(info, card, square);
	return std::make_unique<DefencePetal>(info, card, square);
}

DefencePetal::DefencePetal(SharedInfo& info, CardInfo card, sf::Vector2i square)
	: Petal(info, card), m_square(square) {
	m_sprite.setPosition(MapInfo::getSquareCenter(m_square));
	m_info.defencePetalMap[m_square.x][m_square.y] = this;
}

void DefencePetal::update() {
	// Animation
	updateAnimation();

	// Move
	updatePosition();
}

void DefencePetal::updatePosition() {
	// Rotate
	m_sprite.rotate(sf::degrees(180.f * m_info.dt.asSeconds()));
}

void DefencePetal::onDead() {
	Petal::onDead();

	m_info.defencePetalMap[m_square.x][m_square.y] = nullptr;
}

// Mob petal
std::unique_ptr<MobPetal> MobPetal::create(SharedInfo& info, CardInfo card) {
	return std::make_unique<MobPetal>(info, card);
}

MobPetal::MobPetal(SharedInfo& info, CardInfo card)
	: Petal(info, card, AssetManager::getPetalTexture(TOWER_SUMMON_MOBS.at(card).type)),
	m_mob(TOWER_SUMMON_MOBS.at(card)) {
	float scale = MOB_RARITY_SCALES.at(m_mob.rarity) * 1.5f;
	setScale(scale);

	m_hp = getMobAttribs().hp;
}

void MobPetal::update() {
	// Animation
	updateAnimation();

	// Position
	updatePosition();

	// Kill if out of bounds
	if (m_position <= 0.f)
		kill();
}

void MobPetal::updatePosition() {
	// Movement along path
	float speed = m_info.playerState.buff.apply("speed", getMobAttribs().speed);
	m_position = std::max(0.f, m_position - m_info.dt.asSeconds() * speed);
	updatePathPosition(m_position);
	m_sprite.rotate(sf::degrees(180.f));
}

int MobPetal::getArmor() const {
	return getMobAttribs().armor;
}

int MobPetal::getDamage() const {
	return getMobAttribs().damage;
}

// Web (Defence)
WebPetal::WebPetal(SharedInfo& info, CardInfo card, sf::Vector2i square)
	: DefencePetal(info, card, square) {
	setScale(0.32f * getAttrib("scale"));
	setAlpha(0.9f);
}

void WebPetal::update() {
	DefencePetal::update();

	float delta = getDelta();
	if (delta == 0.f)
		kill();
	else
		setAlpha(0.4f + delta * 0.5f);
}

int WebPetal::getArmor() const {
	return INF;
}

int WebPetal::getDamage() const {
	return 0;
}

void WebPetal::applyDebuff(Debuff& debuff) const {
	const std::string& rarity = getCard().rarity;
	debuff.speed.swap({ getAttrib("slow_down"), rarity, sf::seconds(0.2f) });
}

float WebPetal::getDelta() const {
	float elapsed = m_clock.getElapsedTime().asSeconds();
	float delta = std::max(0.f, 1.f - elapsed / getAttrib("duration"));
	return delta;
}

// Triangle (Shoot)
TrianglePetal::TrianglePetal(SharedInfo& info, CardInfo card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target, int adjCount) 
	: ShootPetal(info, card, startPosition, target), m_adjCount(adjCount) {}

int TrianglePetal::getDamage() const {
	return Petal::getDamage() + (int)getAttrib("damage_increase") * m_adjCount;
}
