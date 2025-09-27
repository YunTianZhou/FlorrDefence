#pragma once
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "Constants.hpp"

class Entity : public sf::Drawable {
public:
    Entity(SharedInfo& info, const sf::Texture& texture);

    void hit(int damage);
    void kill() { hit(m_hp + getArmor()); }
    bool isDead() const { return m_hp <= 0; }
    bool isDeadAnimationDone() const { return m_deathTime <= sf::Time::Zero; }

    virtual void onDead() {}
    virtual void updatePosition() {}
    virtual int getArmor() const { return 0; }
    void updatePathPosition(float position);
    void updateAnimation();

    const sf::Sprite& getSprite() const { return m_sprite; }
    const sf::Texture& getTexture() const { return m_sprite.getTexture(); }
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }

    sf::Angle getRotationOffset() const { return m_rotationOffset; }
    void setRotationOffset(sf::Angle offset) { m_rotationOffset = offset; }
    void rotate(sf::Angle angle) { m_rotationOffset += angle; }

protected:
    void setScale(float scale);
    void setFlash(sf::Color color, float brightness);
    void setAlpha(float alpha);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

public:
    inline static sf::Time flashDuration = sf::seconds(0.15f);
    inline static sf::Time deathDuration = sf::seconds(0.2f);

protected:
    SharedInfo& m_info;
    int m_hp = 0;
    mutable sf::Sprite m_sprite;
    sf::Angle m_rotation = sf::degrees(45.f);
    sf::Angle m_rotationOffset = sf::degrees(0.f);

private:
    float m_scale = 1.f;
    float m_alpha = 1.f;
    sf::Color m_flashColor;
    float m_flashBrightness = 1.f;
    sf::Time m_flashTime;
    sf::Time m_deathTime;
};
