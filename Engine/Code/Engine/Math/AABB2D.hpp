#pragma once
#include "Engine/Math/Vector2.hpp"


class AABB2D
{
public:
	Vector2 mins;
	Vector2 maxs;

	// Composition
	~AABB2D() {};
	AABB2D() {};
	AABB2D(const AABB2D& copy);
	explicit AABB2D(float minX, float minY, float maxX, float maxY);
	explicit AABB2D(const Vector2& mins, const Vector2& maxs);
	explicit AABB2D(const Vector2& center, float radiusX, float radiusY);
	void SetFromText(const char* text);

	//Mutation
	void StretchToIncludePoint(float x, float y); // note: stretch, not move…
	void StretchToIncludePoint(const Vector2& point); // note: stretch, not move…
	void AddPaddingToSides(float xPaddingRadius, float yPaddingRadius);
	void Translate(float translationX, float translationY);
	void Translate(const Vector2& translation); // move the box; similar to +=

	// Accessors
	Vector2 GetDimensions() const; // return a Vector2 of ( width, height )
	Vector2 GetCenter() const; // return the center position of the box

	// Queries
	bool IsPointInside(float x, float y) const; // is “x,y” within box’s interior?
	bool IsPointInside(const Vector2& point) const; // is “point” within box’s interior?

	// Operators
	void operator+=(const Vector2& translation); // move (translate) the box
	void operator-=(const Vector2& translation);
	AABB2D operator+(const Vector2& translation) const; // create a (temp) moved box
	AABB2D operator-(const Vector2& translation) const;

	static const AABB2D ZERO_TO_ONE;
};