#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>

class SpriteCollisionManager {
public:
    static void load();
    static sf::FloatRect getTrimmedBounds(const sf::Texture& texture);
    static sf::FloatRect getTrimmedBounds(const sf::Sprite& sprite);
    static bool isCollide(const sf::Sprite& a, const sf::Sprite& b);

private:
    SpriteCollisionManager() = default;
    static SpriteCollisionManager& getInstance();

    sf::FloatRect _getTrimmedBounds(const sf::Texture& texture);
    const std::vector<bool>& _getAlphaMask(const sf::Texture& texture);
    bool _isCollide(const sf::Sprite& a, const sf::Sprite& b);

private:
    void addTexture(const sf::Texture& texture);

public:
    inline static unsigned char alphaThreshold = 10;

private:
    std::unordered_map<const sf::Texture*, sf::FloatRect> m_trimmedBounds;
    std::unordered_map<const sf::Texture*, std::vector<bool>> m_alphaMasks;
};
