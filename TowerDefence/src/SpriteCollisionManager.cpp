#include "SpriteCollisionManager.hpp"
#include "AssetManager.hpp"

void SpriteCollisionManager::load() {
    // Mobs
    const TexEntry& mobs = AssetManager::getEntry().get("mobs");
    for (const auto& [_, sub] : mobs.subs)
        if (sub->isTexture)
            getInstance().addTexture(*sub->tex);

    // Petals
    const TexEntry& petals = AssetManager::getEntry().get("petals");
    for (const auto& [_, sub] : petals.subs) {
        if (sub->isTexture)
            getInstance().addTexture(*sub->tex);
    }
}

sf::FloatRect SpriteCollisionManager::getTrimmedBounds(const sf::Texture& texture) {
    return getInstance()._getTrimmedBounds(texture);
}

sf::FloatRect SpriteCollisionManager::getTrimmedBounds(const sf::Sprite& sprite) {
    const sf::Texture* tex = &sprite.getTexture();
    return sprite.getTransform().transformRect(getInstance()._getTrimmedBounds(*tex));
}

bool SpriteCollisionManager::isCollide(const sf::Sprite& a, const sf::Sprite& b) {
    return getInstance()._isCollide(a, b);
}

SpriteCollisionManager& SpriteCollisionManager::getInstance() {
    static SpriteCollisionManager instance;
    return instance;
}

sf::FloatRect SpriteCollisionManager::_getTrimmedBounds(const sf::Texture& texture) {
    if (m_trimmedBounds.find(&texture) == m_trimmedBounds.end())
        addTexture(texture);

    return m_trimmedBounds.at(&texture);
}

const std::vector<bool>& SpriteCollisionManager::_getAlphaMask(const sf::Texture& texture) {
    if (m_alphaMasks.find(&texture) == m_alphaMasks.end())
        addTexture(texture);

    return m_alphaMasks.at(&texture);
}

bool SpriteCollisionManager::_isCollide(const sf::Sprite& a, const sf::Sprite& b) {
    const sf::Texture* texA = &a.getTexture();
    const sf::Texture* texB = &b.getTexture();

    sf::IntRect subRectA = a.getTextureRect();
    sf::IntRect subRectB = b.getTextureRect();

    const auto& maskA = _getAlphaMask(*texA);
    const auto& maskB = _getAlphaMask(*texB);

    sf::FloatRect boundsA = getTrimmedBounds(a);
    sf::FloatRect boundsB = getTrimmedBounds(b);

    std::optional optionalIntersection = boundsA.findIntersection(boundsB);
    if (!optionalIntersection)
        return false;
    sf::FloatRect intersection = *optionalIntersection;

    for (int i = int(intersection.position.x); i < int(intersection.position.x + intersection.size.x); i++) {
        for (int j = int(intersection.position.y); j < int(intersection.position.y + intersection.size.y); j++) {
            sf::Vector2f p(i + 0.5f, j + 0.5f);
            sf::Vector2f localA = a.getInverseTransform().transformPoint(p);
            sf::Vector2f localB = b.getInverseTransform().transformPoint(p);

            int xA = int(localA.x + subRectA.position.x);
            int yA = int(localA.y + subRectA.position.y);
            int xB = int(localB.x + subRectB.position.x);
            int yB = int(localB.y + subRectB.position.y);

            if (xA >= 0 && yA >= 0 && xA < int(texA->getSize().x) && yA < int(texA->getSize().y) &&
                xB >= 0 && yB >= 0 && xB < int(texB->getSize().x) && yB < int(texB->getSize().y)) {

                if (maskA[yA * texA->getSize().x + xA] &&
                    maskB[yB * texB->getSize().x + xB]) {
                    return true;
                }
            }
        }
    }

    return false;
}

void SpriteCollisionManager::addTexture(const sf::Texture& texture) {
    sf::Image img = texture.copyToImage();
    unsigned int w = img.getSize().x;
    unsigned int h = img.getSize().y;

    // Trimmed bounds
    unsigned int minX = w, minY = h;
    unsigned int maxX = 0, maxY = 0;
    bool found = false;

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            if (img.getPixel({ x, y }).a > alphaThreshold) {
                if (!found) {
                    minX = maxX = x;
                    minY = maxY = y;
                    found = true;
                }
                else {
                    if (x < minX) minX = x;
                    if (y < minY) minY = y;
                    if (x > maxX) maxX = x;
                    if (y > maxY) maxY = y;
                }
            }
        }
    }

    sf::FloatRect bounds = !found ? sf::FloatRect({ 0, 0 }, { 0, 0 })
        : sf::FloatRect({ float(minX), float(minY) }, { float(maxX - minX + 1), float(maxY - minY + 1) });

    m_trimmedBounds[&texture] = bounds;

    // Alpha mask
    std::vector<bool> alphaMask(w * h);
    for (unsigned y = 0; y < h; y++) {
        for (unsigned x = 0; x < w; x++) {
            alphaMask[y * w + x] = img.getPixel({ x, y }).a > alphaThreshold;
        }
    }

    m_alphaMasks[&texture] = std::move(alphaMask);
}
