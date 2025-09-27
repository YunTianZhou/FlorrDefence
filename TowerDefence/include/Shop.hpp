#pragma once
#include <SFML/Graphics.hpp>
#include "SharedInfo.hpp"
#include "ScrollBar.hpp"
#include "RoundRect.hpp"
#include "Button.hpp"
#include "Card.hpp"

class Product : public sf::Drawable, public sf::Transformable {
public:
	Product();

	void setWidth(float width);
	void setCard(const CardInfo& card);
	void setCount(int count);

	int getPrice() const { return m_price; }
	int getCount() const { return m_stack.getCount(); }
	CardStackInfo getCardStackInfo() const { return m_stack.getInfo(); }

	void update(const SharedInfo& info);
	bool onMouseButtonPressed(const sf::Event::MouseButtonPressed& event);
	bool onMouseButtonReleased(const sf::Event::MouseButtonReleased& event);

private:
	void updatePrice();
	void updateCB() const;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	int m_price = 0;
	sf::RoundRect m_background;
	CardStack m_stack;
	sf::Vector2f m_size;
	sf::Vector2f m_mousePosition;
	mutable ClickButton m_buyCB;
	mutable sf::RectangleShape m_icon;
	mutable bool m_cbUpdated = false;
};

class ShopInfo {
public:
	ShopInfo(const std::string& type);
	bool update(sf::Time now);

	const std::vector<std::string>& getProducts() const { return m_products; }
	std::map<std::string, int>& getCache() { return m_productCountCache; }
	const std::map<std::string, int>& getCache() const { return m_productCountCache; }
	sf::Time getElapsedTime(sf::Time now) const;
	sf::Time getRemainingTime(sf::Time now) const;
	

private:
	void refresh();

private:
	std::string m_type;
	std::vector<std::string> m_products;
	std::map<std::string, int> m_productCountCache;
	sf::Time m_lastRefreshTime;
};

class Shop : public sf::Drawable {
public:
	Shop(SharedInfo& info);

	void update();
	void onEnter();
	void onExit();
	void onEvent(const sf::Event& event);

	void updateComponents() const;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void initComponents();

private:
	inline static const float startX = 1060.f;
	inline static const float startY = 330.f;
	inline static const float endY = 1090.f;
	inline static const float width = 580.f;
	inline static const float productWidth = 100.f;
	inline static const float productHeight = productWidth * (7.f / 6.f);
	inline static const float productSpacing = 20.f;
	inline static const sf::FloatRect subWindowRect = sf::FloatRect({ startX, startY - 10.f }, { width, endY - startY - 10.f });

private:
	SharedInfo& m_info;
	RadioButtonGroup m_menu;
	sf::Text m_elapsedRefreshTimeText;
	std::map<std::string, ShopInfo> m_shops;
	mutable ScrollBar m_scrollBar;
	mutable float m_contentHeight = 0.f;
	mutable std::vector<Product> m_products;
	mutable bool m_updated = false;
};
