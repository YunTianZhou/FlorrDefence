#pragma once
#include <string>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include <SFML/Graphics.hpp>

static inline std::string toNiceString(int x) {
	if (x < 1000)
		return std::to_string(x);
	else if (x < 1000000) {
		int whole = x / 1000;
		int decimal = (x % 1000) / 100;
		return decimal ? std::to_string(whole) + "." + std::to_string(decimal) + "k"
			: std::to_string(whole) + "k";
	}
	else {
		int whole = x / 1000000;
		int decimal = (x % 1000000) / 100000;
		return decimal ? std::to_string(whole) + "." + std::to_string(decimal) + "m"
			: std::to_string(whole) + "m";
	}
}

static inline std::string capitalized(const std::string& word) {
	if (word.empty()) return "";

	std::string result = word;
	result[0] = std::toupper(result[0]);

	for (size_t i = 1; i < result.size(); ++i) {
		result[i] = std::tolower(result[i]);
	}

	return result;
}

static inline std::string toNiceTime(sf::Time duration) {
	int total = (int)duration.asSeconds();
	int days = total / 86400; total %= 86400;
	int hours = total / 3600; total %= 3600;
	int minutes = total / 60; total %= 60;
	int seconds = total;

	std::string result;

	auto append = [&](int value, const char* unit) {
		if (value > 0) {
			result += std::to_string(value) + " " + unit;
			if (value > 1) result += "s";
			result += " ";
		}
		};

	append(days, "day");
	append(hours, "hour");
	append(minutes, "minute");
	append(seconds, "second");

	if (!result.empty() && result.back() == ' ')
		result.pop_back();

	return result;
}

static inline sf::FloatRect getScissorRect(const sf::RenderTarget& target, const sf::FloatRect& coordRect) {
	sf::Vector2u size = target.getSize();
	sf::Vector2i pixelTL = target.mapCoordsToPixel(coordRect.position);
	sf::Vector2i pixelBR = target.mapCoordsToPixel(coordRect.position + coordRect.size);

	sf::Vector2f normTL(
		(float)(pixelTL.x) / size.x,
		(float)(pixelTL.y) / size.y
	);
	sf::Vector2f normBR(
		(float)(pixelBR.x) / size.x,
		(float)(pixelBR.y) / size.y
	);

	sf::FloatRect scissorRect(normTL, normBR - normTL);
	return scissorRect;
}

inline std::mt19937& globalRNG() {
	static std::mt19937 rng{ std::random_device{}() };
	return rng;
}

static inline int randomInt(int low, int high) {
	std::uniform_int_distribution<int> dis(low, high);
	return dis(globalRNG());
}

template<typename T>
T randomUniform(T low, T high) {
	if constexpr (std::is_integral_v<T>) {
		std::uniform_int_distribution<T> dis(low, high);
		return dis(globalRNG());
	}
	else if constexpr (std::is_floating_point_v<T>) {
		std::uniform_real_distribution<T> dis(low, high);
		return dis(globalRNG());
	}
	else {
		static_assert(std::is_arithmetic_v<T>, "uniform<T>: T must be an arithmetic type.");
	}
}

template<typename T>
std::vector<T> randomSample(const std::vector<T>& input, std::size_t k) {
	std::vector<T> result;
	std::sample(input.begin(), input.end(), std::back_inserter(result),
		k, globalRNG());
	return result;
}

