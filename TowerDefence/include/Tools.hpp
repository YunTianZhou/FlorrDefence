#pragma once
#include <string>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include <SFML/Graphics.hpp>

static inline std::string toNiceString(int64_t x) {
	static const char* units[] = { "", "k", "m", "b", "t" };
	static const int N_UNITS = sizeof(units) / sizeof(units[0]);

	if (x > -1000 && x < 1000) return std::to_string(x);

	bool negative = x < 0;
	uint64_t ux = static_cast<uint64_t>(negative ? -x : x);

	uint64_t powers[N_UNITS];
	powers[0] = 1;
	for (int i = 1; i < N_UNITS; ++i) powers[i] = powers[i - 1] * 1000ULL;

	int idx = 0;
	while (idx + 1 < N_UNITS && ux >= powers[idx + 1]) ++idx;

	uint64_t whole = ux / powers[idx];
	uint64_t rem = ux % powers[idx];
	uint64_t decimal = (powers[idx] >= 10) ? (rem / (powers[idx] / 10)) : 0;

	std::string s = (negative ? "-" : "");
	s += std::to_string(whole);
	if (decimal) {
		s += ".";
		s += std::to_string(decimal);
	}
	s += units[idx];
	return s;
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

static inline std::string formatFloat(float value, int precision) {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision) << value;
	std::string str = ss.str();

	if (str.find('.') != std::string::npos) {
		str = str.substr(0, str.find_last_not_of('0') + 1);
		if (str.back() == '.') {
			str.pop_back();
		}
	}

	return str;
}

static inline float getDistanceSquare(sf::Vector2f origin, sf::Vector2f target) {
	float dx = target.x - origin.x;
	float dy = target.y - origin.y;
	float distSquared = dx * dx + dy * dy;

	return distSquared;
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
