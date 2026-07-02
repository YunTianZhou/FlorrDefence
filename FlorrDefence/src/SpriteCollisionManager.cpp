#include "SpriteCollisionManager.hpp"
#include "AssetManager.hpp"

void SpriteCollisionManager::load() {
    const TexEntry& mobs = AssetManager::getEntry().get("mobs");
    for (const auto& [_, sub] : mobs.subs)
        if (sub->isTexture)
            getInstance().addTexture(*sub->tex);

    const TexEntry& petals = AssetManager::getEntry().get("petals");
    for (const auto& [_, sub] : petals.subs)
        if (sub->isTexture)
            getInstance().addTexture(*sub->tex);
}

sf::FloatRect SpriteCollisionManager::getTrimmedBounds(const sf::Texture& texture) {
    return getInstance()._getTrimmedBounds(texture);
}

sf::FloatRect SpriteCollisionManager::getTrimmedBounds(const sf::Sprite& sprite) {
    return sprite.getTransform().transformRect(getInstance()._getTrimmedBounds(sprite.getTexture()));
}

bool SpriteCollisionManager::isCollide(const sf::Sprite& a, const sf::Sprite& b) {
    return getInstance()._isCollide(a, b);
}

SpriteCollisionManager& SpriteCollisionManager::getInstance() {
    static SpriteCollisionManager instance;
    return instance;
}

sf::FloatRect SpriteCollisionManager::_getTrimmedBounds(const sf::Texture& texture) {
    if (!m_trimmedBounds.contains(&texture))
        addTexture(texture);
    return m_trimmedBounds.at(&texture);
}

const std::vector<uint8_t>& SpriteCollisionManager::_getAlphaMask(const sf::Texture& texture) {
    if (!m_alphaMasks.contains(&texture))
        addTexture(texture);
    return m_alphaMasks.at(&texture);
}

bool SpriteCollisionManager::_isCollide(const sf::Sprite& a, const sf::Sprite& b) {
    const sf::Texture* texA = &a.getTexture();
    const sf::Texture* texB = &b.getTexture();

    const auto& maskA = _getAlphaMask(*texA);
    const auto& maskB = _getAlphaMask(*texB);

    const auto sizeA = texA->getSize();
    const auto sizeB = texB->getSize();

    const int wA = (int)sizeA.x, hA = (int)sizeA.y;
    const int wB = (int)sizeB.x, hB = (int)sizeB.y;

    const auto subA = a.getTextureRect();
    const auto subB = b.getTextureRect();

    auto inter = getTrimmedBounds(a).findIntersection(getTrimmedBounds(b));
    if (!inter) return false;

    const sf::FloatRect r = *inter;

    const int stepX = std::clamp((int)(r.size.x / 8.f) - 1, 1, 10);
    const int stepY = std::clamp((int)(r.size.y / 8.f) - 1, 1, 10);

    const auto& invA = a.getInverseTransform();
    const auto& invB = b.getInverseTransform();

    const int sx = (int)r.position.x;
    const int sy = (int)r.position.y;
    const int ex = (int)(r.position.x + r.size.x);
    const int ey = (int)(r.position.y + r.size.y);

    const sf::Vector2f origin((float)sx + 0.5f, (float)sy + 0.5f);

    sf::Vector2f rowA = invA.transformPoint(origin);
    sf::Vector2f rowB = invB.transformPoint(origin);

    const sf::Vector2f dxA = invA.transformPoint({ origin.x + (float)stepX,origin.y }) - rowA;
    const sf::Vector2f dyA = invA.transformPoint({ origin.x,origin.y + (float)stepY }) - rowA;

    const sf::Vector2f dxB = invB.transformPoint({ origin.x + (float)stepX,origin.y }) - rowB;
    const sf::Vector2f dyB = invB.transformPoint({ origin.x,origin.y + (float)stepY }) - rowB;

    for (int y = sy; y < ey; y += stepY) {
        sf::Vector2f localA = rowA;
        sf::Vector2f localB = rowB;

        for (int x = sx; x < ex; x += stepX) {

            const int xA = (int)localA.x + subA.position.x;
            const int yA = (int)localA.y + subA.position.y;

            const int xB = (int)localB.x + subB.position.x;
            const int yB = (int)localB.y + subB.position.y;

            if ((unsigned)xA < sizeA.x && (unsigned)yA < sizeA.y &&
                (unsigned)xB < sizeB.x && (unsigned)yB < sizeB.y) {

                if (maskA[yA * wA + xA] && maskB[yB * wB + xB])
                    return true;
            }

            localA += dxA;
            localB += dxB;
        }

        rowA += dyA;
        rowB += dyB;
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
    std::vector<uint8_t> alphaMask(w * h);
    for (unsigned y = 0; y < h; y++) {
        for (unsigned x = 0; x < w; x++) {
            alphaMask[y * w + x] = img.getPixel({ x, y }).a > alphaThreshold ? 1 : 0;
        }
    }

    m_alphaMasks[&texture] = std::move(alphaMask);
}
