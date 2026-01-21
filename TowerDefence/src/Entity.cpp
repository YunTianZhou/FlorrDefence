#include "Entity.hpp"
#include "AssetManager.hpp"
#include <cmath>
#include <algorithm>

static float angleDelta(float from, float to) {
    float diff = fmodf(to - from, 360.f);
    if (diff < -180.f) diff += 360.f;
    else if (diff > 180.f) diff -= 360.f;
    return diff;
}

Entity::Entity(SharedInfo& info, const sf::Texture& texture)
    : m_info(info), m_sprite(texture)
{
    m_sprite.setTexture(texture);
    m_sprite.setOrigin({ texture.getSize().x / 2.f, texture.getSize().y / 2.f });
}

void Entity::hit(int damage, DamageType type) {
    if (isDead()) return;
    if (type == DamageType::Normal) {
        damage -= getArmor();
        if (damage <= 0) return;
    }
    m_hp -= damage;
    m_flashTime = flashDuration;
    if (isDead())
        m_deathTime = getDeathDuration();
}

void Entity::updateAnimation() {
    if (m_flashTime > sf::Time::Zero)
        m_flashTime -= m_info.dt;
    if (m_deathTime > sf::Time::Zero)
        m_deathTime -= m_info.dt;
}

void Entity::setScale(float scale) {
    m_scale = scale;
    m_sprite.setScale({ scale, scale });
}

void Entity::setFlash(sf::Color color, float brightness) {
    m_flashColor = color;
    m_flashBrightness = brightness;
}

void Entity::setAlpha(float alpha) {
    m_alpha = alpha;
}

void Entity::updatePathPosition(float position) {
    static const sf::Vector2f squareSize{ 100.f, 100.f };
    int numSquares = (int)(PATH_SQUARES.size());

    int i = (int)(std::floor(position - 0.5f - 1e-3f));
    sf::Vector2i g0 = i >= 0 ? PATH_SQUARES[i] : sf::Vector2i(5, -1);
    sf::Vector2i g1 = (i + 1 < numSquares) ? PATH_SQUARES[i + 1] : sf::Vector2i(2, 10);

    sf::Vector2f p0{ g0.y * squareSize.x, g0.x * squareSize.y };
    sf::Vector2f p1{ g1.y * squareSize.x, g1.x * squareSize.y };
    sf::Vector2f dir = p1 - p0;

    float t = position - i - 0.5f;
    sf::Vector2f pos = p0 + dir * t + squareSize / 2.f;
    m_sprite.setPosition(pos);

    if (std::hypot(dir.x, dir.y) > 1e-3f) {
        float targetDeg = std::atan2(dir.y, dir.x) * 180.f / 3.14159265f;
        targetDeg += 135.f;

        float current = m_rotation.asDegrees();
        float delta = angleDelta(current, targetDeg);
        m_rotation = sf::degrees(current + delta * 0.2f);
    }
    m_sprite.setRotation(m_rotationOffset + m_rotation);
}

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    float alpha = m_alpha;
    if (m_deathTime > sf::Time::Zero) {
        float t = m_deathTime.asSeconds() / getDeathDuration().asSeconds();
        float scale = m_scale * (1.f + (1.f - t) * 0.3f);
        alpha *= t;

        m_sprite.setScale({ scale, scale });
    }
    else if (isDead()) {
        return;
    }
    m_sprite.setColor({ 255, 255, 255, unsigned char(255 * alpha) });

    if (m_flashTime > sf::Time::Zero) {
        float strength = std::clamp(m_flashTime.asSeconds() / flashDuration.asSeconds(), 0.f, 1.f);
        float brightness = m_flashBrightness * strength;

        sf::Glsl::Vec4 flashColor = sf::Glsl::Vec4(
            1.f * (1 - strength) + m_flashColor.r / 255.f * strength,
            1.f * (1 - strength) + m_flashColor.g / 255.f * strength,
            1.f * (1 - strength) + m_flashColor.b / 255.f * strength,
            alpha
        );

        sf::Shader& flashShader = AssetManager::getShader("brightness.frag");
        flashShader.setUniform("brightness", brightness);
        flashShader.setUniform("flashColor", flashColor);

        states.shader = &flashShader;
    }

    target.draw(m_sprite, states);
}
