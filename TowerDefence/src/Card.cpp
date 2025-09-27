#include "Card.hpp"
#include "AssetManager.hpp"
#include "Tools.hpp"

Card::Card() {
    m_texRect.setTextureRect({ { 13, 13 }, { 230, 230 } });
}

void Card::setCard(const CardInfo& card) {
    m_card = card;
    m_texRect.setTexture(&AssetManager::getCardTexture(card.type));
    updateColor();
}

void Card::setLength(float length) {
    m_length = length;
    m_texRect.setSize({ length, length });

    m_backgroundRect.setSize({ length, length });
    m_backgroundRect.setOutlineThickness(length * -(77.f / 922.f));
    m_backgroundRect.setRadius(length * (30.f / 922.f));
}

void Card::setDisabled(bool disabled) {
    if (m_disabled == disabled) return;
    m_disabled = disabled;
    updateColor();
}

void Card::updateColor() {
    const std::string key = m_disabled ? "disabled" : m_card.rarity;
    m_backgroundRect.setFillColor(LIGHT_COLORS.at(key));
    m_backgroundRect.setOutlineColor(DARK_COLORS.at(key));
}

void Card::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(m_backgroundRect, states);
    if (m_disabled) 
        states.shader = &AssetManager::getShader("grayscale.frag");
    target.draw(m_texRect, states);
    states.shader = nullptr;
}

CardStack::CardStack()
    : Card()
    , m_text(AssetManager::getFont())
{
    m_text.setFillColor(sf::Color::White);
    m_text.setOutlineColor(sf::Color::Black);
}

void CardStack::setLength(float length) {
    Card::setLength(length);
    m_textUpdated = false;
}

void CardStack::setCount(int count) {
    m_count = count;
    m_textUpdated = false;
}

void CardStack::setInfo(const CardStackInfo& info) {
    setCard(info.card);
    setCount(info.count);
}

void CardStack::updateText() const {
    if (m_length <= 0.f)
        return;

    m_text.setString("x" + toNiceString(m_count));
    m_text.setRotation(sf::degrees(0.f));
    m_text.setCharacterSize((unsigned int)(m_length * 0.28f));
    m_text.setOutlineThickness(m_length * 0.01f);

    float spacing = -m_backgroundRect.getOutlineThickness() + m_length * 0.05f;
    m_text.setOrigin(m_text.getGlobalBounds().size / 2.f + m_text.getLocalBounds().position);
    m_text.setPosition({ m_length - spacing, spacing });
    m_text.setRotation(sf::degrees(24.f));

    m_textUpdated = true;
}

void CardStack::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    Card::draw(target, states);
    
    states.transform *= getTransform();
    if (m_count > 1) {
        if (!m_textUpdated)
            updateText();
        target.draw(m_text, states);
    }
}

void TowerCard::setReload(float reload, bool top) {
    m_reload = reload;
    m_top = top;
}

void TowerCard::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    m_reloadRect.setFillColor(DARK_COLORS.at(getCard().rarity));
    float outline = getLength() * (77.f / 922.f);
    float inside = getLength() - outline * 2;
    m_reloadRect.setSize({ inside, inside * (1.f - m_reload) });
    if (m_top)
        m_reloadRect.setPosition({ outline, outline });
    else
        m_reloadRect.setPosition({ outline, getLength() - outline - m_reloadRect.getSize().y });

    states.transform *= getTransform();
    
    target.draw(m_backgroundRect, states);
    target.draw(m_reloadRect, states);
    target.draw(m_texRect, states);
}
