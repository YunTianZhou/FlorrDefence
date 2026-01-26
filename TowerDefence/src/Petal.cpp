#include "Petal.hpp"
#include "Map.hpp"
#include "Tools.hpp"
#include "AssetManager.hpp"

// Petal
Petal::Petal(SharedInfo& info, const CardInfo& card)
	: m_attribs(TOWER_ATTRIBS[card.type][card.rarity]), m_card(card),
	Entity(info, AssetManager::getPetalTexture(card.type)) {
	setScale(0.32f);
	setFlash(sf::Color(180, 0, 0), 0.4f);
	m_hp = (int)getAttrib("hp");

	m_info.counter.petal[card]++;
}

Petal::Petal(SharedInfo& info, const CardInfo& card, const sf::Texture& texture)
	: m_attribs(TOWER_ATTRIBS[card.type][card.rarity]), m_card(card),
	Entity(info, texture) {
	setFlash(sf::Color(180, 0, 0), 0.4f);

	m_info.counter.petal[card]++;
}

int Petal::getArmor() const {
	return hasAttrib("armor") ? int(getAttrib("armor")) : 0;
}

int Petal::getDamage() const {
	return int(getBuffedAttrib("damage"));
}

void Petal::onDead() {
	if (hasAttrib("death_heal"))
		m_info.playerState.heal(getAttrib("death_heal"));

	m_info.counter.petal[getCard()]--;
}

// ShootPetal
std::unique_ptr<ShootPetal> ShootPetal::create(SharedInfo& info, const CardInfo& card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target) {
	if (card.type == "lightning")
		return std::make_unique<LightningPetal>(info, card, startPosition, target);
	else if (card.type == "pincer")
		return std::make_unique<PincerPetal>(info, card, startPosition, target);
	else if (card.type == "dice")
		return std::make_unique<DicePetal>(info, card, startPosition, target);
	else if (card.type == "bur")
		return std::make_unique<BurPetal>(info, card, startPosition, target);
	else if (card.type == "chip")
		return std::make_unique<ChipPetal>(info, card, startPosition, target);
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
	{"triangle", sf::degrees(-90.f)},
	{"lightning", sf::degrees(0.f)},
	{"pincer", sf::degrees(40.f)},
	{"dice", sf::degrees(-38.f)},
	{"bur", sf::degrees(0.f)},
	{"laser", sf::degrees(90.f)},
	{"chip", sf::degrees(0.f)},
	{"rice", sf::degrees(40.f)}
};

ShootPetal::ShootPetal(SharedInfo& info, const CardInfo& card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target)
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

ShootPetal::ShootPetal(SharedInfo& info, const CardInfo& card) 
	: Petal(info, card) {}

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
	float range = m_info.playerState.buff.reach.apply(getAttrib("range") * MapInfo::squareSize.x);
	if (dstSquare > range * range) {
		kill();
		return;
	}
}

void ShootPetal::updatePosition() {
	// Direction
	updateDirection(720.f);

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

void ShootPetal::updateDirection(float trunSpeed) {
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

	float turnRadPerSec = trunSpeed * (PI / 180.f);
	float maxStep = turnRadPerSec * m_info.dt.asSeconds();
	
	float step = std::max(-maxStep, std::min(maxStep, diff));
	float next = current + step;
	m_direction = sf::radians(next);

	m_sprite.setRotation(m_direction + sf::degrees(90.f) - petalTilt.at(getCard().type));
}

// DefencePetal
std::unique_ptr<DefencePetal> DefencePetal::create(SharedInfo& info, const CardInfo& card, sf::Vector2i square) {
	if (card.type == "web")
		return std::make_unique<WebPetal>(info, card, square);
	else if (card.type == "jelly")
		return std::make_unique<JellyPetal>(info, card, square);
	return std::make_unique<DefencePetal>(info, card, square);
}

DefencePetal::DefencePetal(SharedInfo& info, const CardInfo& card, sf::Vector2i square)
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
std::unique_ptr<MobPetal> MobPetal::create(SharedInfo& info, const CardInfo& card) {
	return std::make_unique<MobPetal>(info, card, 39.f + randomUniform(-0.3f, 0.f));
}

MobPetal::MobPetal(SharedInfo& info, const CardInfo& card, float startPosition)
	: Petal(info, card, AssetManager::getPetalTexture(TOWER_SUMMON_MOBS.at(card).type)),
	m_mob(TOWER_SUMMON_MOBS.at(card)), m_position(startPosition) {
	float scale = MOB_RARITY_SCALES.at(m_mob.rarity) * 1.5f;
	setScale(scale);

	m_hp = (int)m_info.playerState.buff.summoner.apply((float)getMobAttribs().hp);
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
	float speed = m_info.playerState.buff.speed.apply(getMobAttribs().speed);
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
WebPetal::WebPetal(SharedInfo& info, const CardInfo& card, sf::Vector2i square)
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
	debuff.webSpeed.swap({ getAttrib("slow_down"), rarity, sf::seconds(0.2f) });
}

float WebPetal::getDelta() const {
	float elapsed = m_clock.getElapsedTime().asSeconds();
	float delta = std::max(0.f, 1.f - elapsed / getAttrib("duration"));
	return delta;
}

// Triangle (Shoot)
TrianglePetal::TrianglePetal(SharedInfo& info, const CardInfo& card, sf::Vector2f startPosition, std::list<std::unique_ptr<Mob>>::const_iterator target, int adjCount)
	: ShootPetal(info, card, startPosition, target), m_adjCount(adjCount) {
}

int TrianglePetal::getDamage() const {
	return Petal::getDamage() + (int)getAttrib("damage_increase") * m_adjCount;
}

// Lightning (Shoot)
void LightningPetal::onHit(Mob& mob, std::list<std::unique_ptr<Mob>>& mobs, std::list<std::unique_ptr<Effect>>& effects) {
	using MobsIt = std::list<std::unique_ptr<Mob>>::iterator;

	std::vector<MobsIt> targets = getTargets(mobs);
	for (MobsIt& target : targets)
		(*target)->hit(getDamage(), getDamageType());


	std::sort(targets.begin(), targets.end(), [](const MobsIt& a, const MobsIt& b) {
		return (*a)->getPathPosition() < (*b)->getPathPosition();
		});

	int connected = -1;
	float connectedDst = 0.f;
	std::vector<sf::Vector2f> positions(targets.size());
	for (int i = 0; i < targets.size(); i++) {
		auto& mob = *targets[i];
		positions[i] = mob->getPosition();

		float dst = abs(mob->getPathPosition() - mob->getPathPosition());
		if (connected == -1 || dst < connectedDst) {
			connected = i;
			connectedDst = dst;
		}
	}

	effects.emplace_back(std::make_unique<LightningEffect>(m_info, getPosition(), connected, move(positions)));
	kill();
}


std::vector<std::list<std::unique_ptr<Mob>>::iterator> LightningPetal::getTargets(std::list<std::unique_ptr<Mob>>& mobs) const {
	const float range = getAttrib("bounce_range") * MapInfo::squareSize.x;
	const float rangeSq = range * range;
	const int maxTargets = (int)(getAttrib("bounces"));
	auto position = getPosition();

	std::vector<std::pair<
		std::list<std::unique_ptr<Mob>>::iterator,
		float>> best;

	for (auto it = mobs.begin(); it != mobs.end(); it++) {
		auto mp = (*it)->getPosition();
		float dx = mp.x - position.x;
		float dy = mp.y - position.y;
		float d2 = dx * dx + dy * dy;
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

	std::vector<std::list<std::unique_ptr<Mob>>::iterator> targets;
	targets.reserve(best.size());
	for (auto& p : best)
		targets.push_back(p.first);

	return targets;
}

// Pincer (Shoot)
void PincerPetal::applyDebuff(Debuff& debuff) const {
	const std::string& rarity = getCard().rarity;
	debuff.pincerSpeed.swap({ getAttrib("slow_down"), rarity, sf::seconds(getAttrib("slow_down_duration")) });
}

// Jelly (Defence)
void JellyPetal::applyDebuff(Debuff& debuff) const {
	const std::string& rarity = getCard().rarity;
	debuff.knockback.swap({ getAttrib("knockback"), rarity });
}

// Dice (Shoot)
int DicePetal::getDamage() const {	
	if (randomUniform(0.f, 1.f) <= boostProb)
		return ShootPetal::getDamage() * boostRate;
	else
		return ShootPetal::getDamage();
}

// Bur (Shoot)
void BurPetal::applyDebuff(Debuff& debuff) const {
	const std::string& rarity = getCard().rarity;
	debuff.armor.swap({ getAttrib("armor_debuff"), rarity, sf::seconds(getAttrib("armor_debuff_duration")) });
}

// Laser
LaserPetal::LaserPetal(SharedInfo& info, const CardInfo& card, sf::Vector2i square, MapInfo& map, const std::list<std::unique_ptr<Mob>>& mobs) 
	: ShootPetal(info, card), m_square(square), m_map(map), m_mobs(mobs) {

	sf::Vector2f texSize = sf::Vector2f(m_sprite.getTexture().getSize());

	float length = MapInfo::squareSize.x;
	float half = texSize.y / 2;
	float scale = length * (getAttrib("range") + 0.5f) / (texSize.x - half);
	setScale(scale);

	m_sprite.setOrigin({ half, half });
	m_sprite.setPosition(MapInfo::getSquareCenter(square));

	m_direction = sf::degrees(randomUniform(0.f, 360.f))  - sf::degrees(90.f);
	m_sprite.setRotation(m_direction + sf::degrees(90.f) - petalTilt.at(getCard().type));

	m_info.laserMap[m_square.x][m_square.y] = true;
}

int LaserPetal::getArmor() const {
	return INF;
}

int LaserPetal::getDamage() const {
	int damage = (int)getBuffedAttrib("damage");

	if (m_target.has_value()) {
		float elapsed = m_clock.getElapsedTime().asSeconds();
		float maxRate = getAttrib("max_damage_rate");
		float increaseRate = std::min(maxRate, 1.f + getAttrib("damage_increase_rate") * elapsed);
		damage = int(damage * increaseRate);
	}

	return damage;
}

void LaserPetal::onDead() {
	ShootPetal::onDead();

	m_info.laserMap[m_square.x][m_square.y] = false;
}

void LaserPetal::update() {
	// If dead
	const Tower* tower = m_map.getTower(m_square);
	if (tower == nullptr || tower->getCard() != getCard()) {
		kill();
		return;
	}

	// Direction
	updateDirection(180.f);

	// Target
	updateTarget();

	if (m_target.has_value())
		m_sprite.setTexture(AssetManager::getPetalTexture("laser"));
	else
		m_sprite.setTexture(AssetManager::getPetalTexture("laser_idle"));
}

void LaserPetal::updatePosition() {}

void LaserPetal::updateTarget() {
	float range = getAttrib("range") * MapInfo::squareSize.x;
	float rangeSquared = range * range;
	auto position = getPosition();

	if (m_target.has_value()) {
		float distSquared = getDistanceSquare(position, m_target.value()->get()->getPosition());
		if (distSquared <= rangeSquared)
			return;
		lostTarget();
	}

	std::optional<std::list<std::unique_ptr<Mob>>::const_iterator> nearestMob;
	float nearestDistSquared = std::numeric_limits<float>::max();

	for (auto it = m_mobs.begin(); it != m_mobs.end(); it++) {
		float distSquared = getDistanceSquare(position, it->get()->getPosition());

		if (distSquared <= rangeSquared && distSquared < nearestDistSquared) {
			nearestDistSquared = distSquared;
			nearestMob = it;
		}
	}

	if (nearestMob.has_value()) {
		m_target = nearestMob;
		m_clock.restart();
	}
}

// Chip Petal
int ChipPetal::getArmor() const {
	if (randomUniform(0.f, 1.f) <= getAttrib("evasion"))
		return INF;
	else
		return 0;
}
