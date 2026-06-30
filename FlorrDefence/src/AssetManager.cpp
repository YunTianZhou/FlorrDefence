#include <iostream>
#include <cassert>
#include "AssetManager.hpp"

static void buildEntries(TexEntry& node, const std::filesystem::path& path) {
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        const auto& p = entry.path();
        std::string key = p.stem().string();

        if (entry.is_directory()) {
            if (entry.path().filename().string().substr(0, 3) == "old")
                continue;
            auto child = std::make_unique<TexEntry>();
            child->isTexture = false;
            buildEntries(*child, p);
            node.subs.emplace(p.filename().string(), std::move(child));
        }
        else if (entry.is_regular_file()) {
            std::string ext = p.extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                auto child = std::make_unique<TexEntry>();
                child->isTexture = true;
                child->tex = std::make_unique<sf::Texture>();
                if (!child->tex->loadFromFile(p.string()))
                    throw std::runtime_error("Texture load failure: " + p.string());
                node.subs.emplace(key, std::move(child));
            }
        }
    }
}

static void loadShaders(std::unordered_map<std::string, sf::Shader>& shaders, const std::filesystem::path& path) {
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().string();
        std::string ext = entry.path().extension().string();
        if (ext == ".frag") {
            shaders.emplace(name, sf::Shader(entry.path(), sf::Shader::Type::Fragment));
        }
        else if (ext == ".vert") {
            shaders.emplace(name, sf::Shader(entry.path(), sf::Shader::Type::Vertex));
        }
        else {
            throw std::runtime_error("Unknown shader type: " + ext);
        }
    }
}

TexEntry::operator const sf::Texture& () const {
    assert(isTexture && tex && "Not a texture entry");
    return *tex;
}

const TexEntry& TexEntry::operator[](const std::string& name) const {
    auto it = subs.find(name);
    assert(it != subs.end() && "Texture entry not found");
    return *it->second;
}

AssetManager::AssetManager() {
    m_entry.isTexture = false;
    buildEntries(m_entry, std::filesystem::path("res/images"));
    loadShaders(m_shaders, std::filesystem::path("res/shaders"));

    if (!m_font.openFromFile("res/fonts/Ubuntu-Bold.ttf"))
        throw std::runtime_error("Font load failure");
}

