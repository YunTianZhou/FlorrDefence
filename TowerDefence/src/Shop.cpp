#include "Shop.hpp"
#include "AssetManager.hpp"
#include "Tools.hpp"

static inline sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
	return {
		(unsigned char)(a.r + (b.r - a.r) * t),
		(unsigned char)(a.g + (b.g - a.g) * t),
		(unsigned char)(a.b + (b.b - a.b) * t),
		(unsigned char)(a.a + (b.a - a.a) * t)
	};
}

Product::Product(SharedInfo& info)
	: m_info(info) {
	m_background.setFillColor(DARK_COLORS.at("wood"));
	m_buyCB.setFillColor(LIGHT_COLORS.at("gold"));
	m_buyCB.setOutline(DARK_COLORS.at("gold"), 0.f);
	m_icon.setTexture(&AssetManager::getUITexture("coin"));
	m_icon.setTextureRect(sf::IntRect({ { 320, 320 }, { 383, 383 } }));
}

void Product::setWidth(float width) {
	m_size = { width, width * (7.f / 6.f) };

	m_background.setSize(m_size);
	m_background.setRadius(m_size.x * 0.08f);

	m_stack.setLength(m_size.x * (2.f / 3.f));
	m_stack.setPosition({ m_size.x / 6.f, m_size.y / 14.f });

	m_cbUpdated = false;
}

void Product::setCard(const CardInfo& card) {
	m_stack.setCard(card);
	m_stack.setCount(1);
	updatePrice();
}

void Product::setCount(int count) {
	if (m_stack.getCard().rarity == "unique")
		count = std::min(count, 1);
	m_stack.setCount(count);
	updatePrice();
}

void Product::update() {
	m_mousePosition = getInverseTransform().transformPoint(m_info.mouseWorldPosition);
	m_buyCB.setDisabled(m_price > m_info.playerState.coin ||
		(m_stack.getCard().rarity == "unique" && m_info.playerState.boughtUniques.count(m_stack.getCard().type)));
	m_buyCB.update(m_mousePosition);

	if (!m_info.draggedCard.has_value() && m_stack.getRect().contains(m_mousePosition)) {
		float length = m_stack.getLength();
		sf::Vector2f pos = getTransform().transformPoint(m_stack.getPosition());
		pos += sf::Vector2f(length, length) / 2.f;
		m_info.cardDescription.set(m_stack.getCard(), pos, length);
	}
}

bool Product::onMouseButtonPressed(const sf::Event::MouseButtonPressed& event) {
	m_buyCB.onMouseButtonPressed(event);

	if (event.button == sf::Mouse::Button::Left && m_stack.getRect().contains(m_mousePosition)) {
		static std::vector<int> counts = { 1, 5, 10, 50, 100, 500, 1000, 1 };
		int index = (int)std::distance(counts.begin(), std::find(counts.begin(), counts.end(), m_stack.getCount()));
		setCount(counts[index + 1]);
		return true;
	}
	return false;
}

bool Product::onMouseButtonReleased(const sf::Event::MouseButtonReleased& event) {
	return m_buyCB.onMouseButtonReleased(event);
}

void Product::updatePrice() {
	m_price = TOWER_ATTRIBS[m_stack.getCard().type][m_stack.getCard().rarity].price * m_stack.getCount();
	m_buyCB.setString(toNiceString(m_price));

	m_cbUpdated = false;
}

void Product::updateCB() const {
	assert(m_size != sf::Vector2f());

	sf::Vector2f cbPosition = { m_size.x * 0.12f, m_size.x / 10.f + m_size.y / 14.f + m_stack.getLength() };
	sf::Vector2f cbSize = { m_size.x * 0.76f, m_size.y * 0.185f };
	m_buyCB.setPosition(cbPosition);
	m_buyCB.setSize(cbSize);
	m_buyCB.setOutline(DARK_COLORS.at("gold"), m_size.x / 30.f);
	m_buyCB.setCharactorSize((unsigned int)(m_size.x * 0.15f));

	float textWidth = m_buyCB.getTextBounds().size.x;
	float offset = (cbSize.x - textWidth - cbSize.y) / 2.f;
	m_buyCB.setTextOffset({ offset + cbSize.y + textWidth / 2.f - cbSize.x / 2.f, 0.f });

	m_icon.setPosition(cbPosition + sf::Vector2f(offset, cbSize.y * 0.1f));
	m_icon.setSize({ cbSize.y * 0.8f, cbSize.y * 0.8f });

	m_cbUpdated = true;
}

void Product::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform = getTransform();

	if (!m_cbUpdated)
		updateCB();

	target.draw(m_background, states);
	target.draw(m_stack, states);
	target.draw(m_buyCB, states);
	if (m_buyCB.isDisabled())
		states.shader = &AssetManager::getShader("grayscale.frag");
	target.draw(m_icon, states);
	states.shader = nullptr;
}

ShopInfo::ShopInfo(const SharedInfo& info, const std::string& type)
	: m_info(info), m_type(type) {
	refresh();
}

bool ShopInfo::update() {
	m_refreshTimer += m_info.dt;

	if (m_refreshTimer >= SHOP_ATTRIBS[m_type].refreshInterval) {
		refresh();
		m_refreshTimer = sf::Time::Zero;
		return true;
	}

	return false;
}

sf::Time ShopInfo::getRemainingTime() const {
	sf::Time interval = SHOP_ATTRIBS[m_type].refreshInterval;

	if (m_refreshTimer >= interval)
		return sf::seconds(0);  // Already expired

	return interval - m_refreshTimer;
}

void ShopInfo::refresh() {
	m_products = randomSample(TOWER_TYPES, SHOP_ATTRIBS[m_type].productCount);
}

Shop::Shop(SharedInfo& info)
	: m_info(info), m_elapsedRefreshTimeText(AssetManager::getFont()) {
	initComponents();
}

void Shop::update() {
	// Menu
	for (int i = 0; i < m_menu.getSize(); i++) {
		bool disable = i > m_info.playerState.buff.shop.apply(0);
		m_menu.getButton(i).setDisabled(disable);

		if (!disable)
			if (m_shops.at(SHOP_RARITIES[i]).update())
				m_updated = false;
	}
	m_menu.update(m_info.mouseWorldPosition);

	// ShopInfo
	updateShopInfo();

	// Elapsed Refresh Time Text
	ShopInfo& shop = m_shops.at(m_menu.getVar());
	sf::Time remain = shop.getRemainingTime();
	float remSec = remain.asSeconds();
	if (remSec < 1.f) {
		m_elapsedRefreshTimeText.setString("Changing store...");
	}
	else {
		std::string remainingTime = toNiceTime(remain);
		m_elapsedRefreshTimeText.setString("Store will change in " + remainingTime);
	}
	m_elapsedRefreshTimeText.setOrigin(m_elapsedRefreshTimeText.getGlobalBounds().size / 2.f +
		m_elapsedRefreshTimeText.getLocalBounds().position);

	if (remSec < warningThreshold.asSeconds()) {
		float phase = std::fmod(warningThreshold.asSeconds() - remSec, 1.0f);
		float t = phase < 0.5f
			? (phase * 2.0f)
			: ((1.0f - phase) * 2.0f);
		sf::Color c = lerpColor(sf::Color::White, sf::Color::Red, t);
		m_elapsedRefreshTimeText.setFillColor(c);
	}
	else {
		m_elapsedRefreshTimeText.setFillColor(sf::Color::White);
	}

	// Products
	sf::Vector2f mousePosition = m_info.mouseWorldPosition;
	if (!subWindowRect.contains(mousePosition))
		mousePosition = { -1.f, -1.f };  // Outside of sub window
	for (Product& product : m_products)
		product.update();

	// Scroll Bar
	float prevOffset = m_scrollBar.getOffset();
	m_scrollBar.update(m_info.mouseWorldPosition);
	if (prevOffset != m_scrollBar.getOffset())
		m_updated = false;
}

void Shop::updateShopInfo() {
	for (int i = 0; i < m_menu.getSize(); i++) {
		bool disable = i > m_info.playerState.buff.shop.apply(0);
		if (!disable)
			if (m_shops.at(SHOP_RARITIES[i]).update())
				m_updated = false;
	}
}

void Shop::onEnter() {
	m_scrollBar.setOffset(0.f);
	m_scrollBar.setContentHeight(0.f);
	m_updated = false;
}

void Shop::onExit() {

}

void Shop::onEvent(const sf::Event& event) {
	if (const auto* pressedEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
		m_menu.onMouseButtonPressed(*pressedEvent);
		m_scrollBar.onMouseButtonPressed(*pressedEvent);

		auto& cache = m_shops.at(m_menu.getVar()).getCache();
		for (Product& product : m_products)
			if (product.onMouseButtonPressed(*pressedEvent))
				cache[product.getCardStackInfo().card.type] = product.getCount();
	}
	else if (const auto* releasedEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (m_menu.onMouseButtonReleased(*releasedEvent)) {
			m_scrollBar.setOffset(0.f);
			m_updated = false;
		}
		m_scrollBar.onMouseButtonReleased(*releasedEvent);
		for (Product& product : m_products) {
			if (product.onMouseButtonReleased(*releasedEvent)) {
				// Buy tower
				CardStackInfo info = product.getCardStackInfo();
				int64_t count = m_info.input.keyShift && info.card.rarity != "unique" ?
					m_info.playerState.coin / product.getPrice() : 1;

				int64_t rem = std::max(0LL, towerLimit - m_info.playerState.backpack.getCount(info.card));
				count = std::min(count, rem / product.getCount());
				assert(count < INT_MAX);  // TEST

				m_info.playerState.coin -= count * product.getPrice();
				info.count *= int(count);

				m_info.playerState.backpack.add(info);
				if (product.getCardStackInfo().card.rarity == "unique")
					m_info.playerState.boughtUniques.insert(product.getCardStackInfo().card.type);
			}
		}
	}
	else if (const auto* scrolledEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
		if (!m_info.input.mouseLeftButton && subWindowRect.contains(m_info.mouseWorldPosition)) {
			m_scrollBar.onMouseWheelScrolled(*scrolledEvent);
			m_updated = false;
		}
	}
}

void Shop::updateComponents() const {
	m_products.clear();

	const std::string& shopType = m_menu.getVar();
	const ShopInfo& shopInfo = m_shops.at(shopType);
	const std::vector<std::string>& productTypes = shopInfo.getProducts();
	const auto& cache = shopInfo.getCache();

	Product product(m_info);
	float y = startY - m_scrollBar.getOffset();
	product.setWidth(productWidth);
	for (int i = 0; i < productTypes.size(); i += 5) {
		int count = std::min(5, (int)productTypes.size() - i);
		float x = startX + (width - count * productWidth - (count - 1) * productSpacing) / 2.f;
		if (y + productHeight > startY && y <= endY) {
			for (int j = 0; j < count; j++) {
				product.setCard(CardInfo(shopType, productTypes[i + j]));
				product.setPosition({ x, y });
				auto it = cache.find(productTypes[i + j]);
				if (it != cache.end())
					product.setCount(it->second);
				product.update();

				m_products.push_back(product);
				x += productWidth + productSpacing;
			}
		}
		y += productHeight + productSpacing;
	}

	y -= productSpacing;
	m_contentHeight = y - (startY - m_scrollBar.getOffset());
	m_scrollBar.setContentHeight(m_contentHeight);
	m_updated = true;
}

void Shop::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(m_menu);
	target.draw(m_elapsedRefreshTimeText);

	sf::View view = target.getView();
	view.setScissor(getScissorRect(target, subWindowRect));
	target.setView(view);

	if (!m_updated)
		updateComponents();

	for (const Product& product : m_products)
		target.draw(product, states);

	view.setScissor({ { 0.f, 0.f }, { 1.f, 1.f } });
	target.setView(view);

	if (m_contentHeight > m_scrollBar.getViewHeight())
		target.draw(m_scrollBar, states);
}

void Shop::initComponents() {
	// Menu and Shop Info
	float menuRBWidth = 100.f;
	float menuSpacing = 20.f;
	float menuRBOutline = 4.f;
	RadioButton menuRB;
	menuRB.setPosition({ 1060.f + menuRBOutline, 250.f });
	menuRB.setSize({ menuRBWidth - menuRBOutline * 2.f, 40.f - menuRBOutline * 2.f });
	menuRB.setCharactorSize(18);

	m_menu.setVar("common");
	for (const std::string& shopType : SHOP_RARITIES) {
		menuRB.setFillColor(LIGHT_COLORS.at(shopType));
		menuRB.setOutline(DARK_COLORS.at(shopType), menuRBOutline);
		menuRB.setString(shopType);
		menuRB.setIdentifier(shopType);
		m_menu.addButton(menuRB);
		menuRB.move({ menuRBWidth + menuSpacing, 0.f });

		m_shops.emplace(shopType, ShopInfo(m_info, shopType));
	}

	// Elapsed time text
	m_elapsedRefreshTimeText.setCharacterSize(15);
	m_elapsedRefreshTimeText.setFillColor(sf::Color::White);
	m_elapsedRefreshTimeText.setOutlineColor(sf::Color::Black);
	m_elapsedRefreshTimeText.setOutlineThickness(1.f);
	m_elapsedRefreshTimeText.setPosition({ 1350.f, 305.f });

	// Scroll bar
	m_scrollBar.setPosition({ 1640.f + 20.f, subWindowRect.position.y });
	m_scrollBar.setSize({ 10.f, subWindowRect.size.y });
	m_scrollBar.setFillColor(DARK_COLORS.at("wood"));
	m_scrollBar.setViewHeight(subWindowRect.size.y - 10.f);
}
