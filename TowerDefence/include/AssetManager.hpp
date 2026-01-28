#pragma once
#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <SFML/Graphics.hpp>
#include "Constants.hpp"

struct TexEntry {
    bool isTexture = false;
    std::unique_ptr<sf::Texture> tex;
    std::unordered_map<std::string, std::unique_ptr<TexEntry>> subs;

    operator const sf::Texture& () const;
    const TexEntry& operator[](const std::string& name) const;

    template<typename... Args>
    const TexEntry& get(const std::string& name, const Args&... rest) const {
        auto it = subs.find(name);
        if (it == subs.end()) {
             throw std::runtime_error("TexEntry not found: " + name);
        }
        if constexpr (sizeof...(rest) == 0) {
            return *it->second;
        }
        else {
            return it->second->get(rest...);
        }
    }
};

class AssetManager {
public:
    static void load() {
        getInstance();
    }

    template<typename... Args>
    static const sf::Texture& getTexture(const Args&... names) {
        return getInstance().m_entry.get(names...);
    }

    template<typename... Args>
    static const sf::Texture& getUITexture(const Args&... names) {
        return getTexture("ui", names...);
    }

    template<typename... Args>
    static const sf::Texture& getCardTexture(const Args&... names) {
        return getTexture("cards", names...);
    }

    template<typename... Args>
    static const sf::Texture& getMobTexture(const Args&... names) {
        return getTexture("mobs", names...);
    }

    template<typename... Args>
    static const sf::Texture& getPetalTexture(const Args&... names) {
        return getTexture("petals", names...);
    }

    template<typename... Args>
    static const sf::Texture& getTalentTexture(const Args&... names) {
        return getTexture("ui", "talents", names...);
    }

    static const sf::Font& getFont() {
        return getInstance()._getFont();
    }

    static sf::Shader& getShader(const std::string& name) {
        return getInstance()._getShader(name);
    }

    static const TexEntry& getEntry() {
        return getInstance().m_entry;
    }

private:
    AssetManager();

    static AssetManager& getInstance() {
        static AssetManager instance;
        return instance;
    }

    const sf::Font& _getFont() const { return m_font; }
    sf::Shader& _getShader(const std::string& name) { return m_shaders.at(name); };

private:
    TexEntry m_entry;
    sf::Font m_font;
    std::unordered_map<std::string, sf::Shader> m_shaders;
};
