#pragma once
#include "Engine/Math/Vector2.hpp"

class Disc2D
{
public:
	Vector2 center;
	float radius;

	// Composition
	~Disc2D() {};
	Disc2D() {};
	Disc2D(const Disc2D& copyFrom);
	explicit Disc2D(float initialX, float initialY, float initialRadius);
	explicit Disc2D(const Vector2& initialCenter, float initialRadius);

	// Mutation
	void StretchToIncludePoint(float x, float y); // expand radius if (x,y) is outside
	void StretchToIncludePoint(const Vector2& point); // expand radius if point outside
	void AddPaddingToRadius(float paddingRadius);
	void Translate(const Vector2& translation); // move the center
	void Translate(float translationX, float translationY); // move the center

	// Queries
	bool IsPointInside(float x, float y) const; // is (x,y) within disc’s interior?
	bool IsPointInside(const Vector2& point) const; // is “point” within disc’s interior?

	// Operators
	void operator+=(const Vector2& translation); // move
	void operator-=(const Vector2& translation);
	Disc2D operator+(const Vector2& translation) const; // create a moved copy
	Disc2D operator-(const Vector2& antiTranslation) const;
};
