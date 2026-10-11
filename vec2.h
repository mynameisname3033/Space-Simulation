#pragma once

#include <cmath>

struct Vec2
{
	double x = 0.0;
	double y = 0.0;

	Vec2 operator+(const Vec2& other) const { return { x + other.x, y + other.y }; }
	Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }

	Vec2 operator-(const Vec2& other) const { return { x - other.x, y - other.y }; }
	Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }

	Vec2 operator*(double scalar) const { return { x * scalar, y * scalar }; }
	Vec2& operator*=(double scalar) { x *= scalar; y *= scalar; return *this; }

	Vec2 operator/(double scalar) const { return { x / scalar, y / scalar }; }

	inline double length_squared() const { return x * x + y * y; }
	inline double length() const { return std::sqrt(length_squared()); }
};