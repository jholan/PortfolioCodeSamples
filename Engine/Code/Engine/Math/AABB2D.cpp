#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2D.hpp"

#include "Engine/Core/StringUtils.hpp"



//-------------------------------------------------------------------------------
AABB2D::AABB2D(const AABB2D& copy)
	: mins(copy.mins)
	, maxs(copy.maxs)
{
}


//-------------------------------------------------------------------------------
AABB2D::AABB2D(float minX, float minY, float maxX, float maxY)
	: mins(minX, minY)
	, maxs(maxX, maxY)
{
}


//-------------------------------------------------------------------------------
AABB2D::AABB2D(const Vector2& mins, const Vector2& maxs)
	: mins(mins)
	, maxs(maxs)
{
}


//-------------------------------------------------------------------------------
AABB2D::AABB2D(const Vector2& center, float radiusX, float radiusY)
	: mins(center.x - radiusX, center.y - radiusY)
	, maxs(center.x + radiusX, center.y + radiusY)
{
}


//-------------------------------------------------------------------------------
void AABB2D::SetFromText(const char* text)
{
	// We must have 3 commas
	const char* comma1 = FindFirstCharacterInString(text, ",");
	if (comma1 == nullptr) { return; }

	const char* comma2 = FindFirstCharacterInString(comma1 + 1, ",");
	if (comma2 == nullptr) { return; }

	const char* comma3 = FindFirstCharacterInString(comma2 + 1, ",");
	if (comma3 == nullptr) { return; }

	mins.x = StringToFloat(text);
	mins.y = StringToFloat(comma1 + 1);
	maxs.x = StringToFloat(comma2 + 1);
	maxs.y = StringToFloat(comma3 + 1);


}


//-------------------------------------------------------------------------------
void AABB2D::StretchToIncludePoint(float x, float y)
{
	mins.x = Min(mins.x, x);
	mins.y = Min(mins.y, y);

	maxs.x = Max(maxs.x, x);
	maxs.y = Max(maxs.y, y);
}


//-------------------------------------------------------------------------------
void AABB2D::StretchToIncludePoint(const Vector2& point)
{
	StretchToIncludePoint(point.x, point.y);
}


//-------------------------------------------------------------------------------
void AABB2D::AddPaddingToSides(float xPaddingRadius, float yPaddingRadius)
{
	mins.x -= xPaddingRadius;
	maxs.x += xPaddingRadius;

	mins.y -= yPaddingRadius;
	maxs.y += yPaddingRadius;
}


//-------------------------------------------------------------------------------
void AABB2D::Translate(const Vector2& translation)
{
	mins += translation;
	maxs += translation;
}


//-------------------------------------------------------------------------------
void AABB2D::Translate(float translationX, float translationY)
{
	Vector2 translationVector = Vector2(translationX, translationY);
	Translate(translationVector);
}


//-------------------------------------------------------------------------------
Vector2 AABB2D::GetDimensions() const
{
	Vector2 dimensions = Vector2(maxs.x - mins.x, maxs.y - mins.y);
	return dimensions;
}


//-------------------------------------------------------------------------------
Vector2 AABB2D::GetCenter() const
{
	Vector2 halfDimensions = GetDimensions() / 2.0f;
	Vector2 center = Vector2(mins.x + halfDimensions.x, mins.y + halfDimensions.y);
	return center;
}


//-------------------------------------------------------------------------------
bool AABB2D::IsPointInside(float x, float y) const
{
	bool isInside = false;

	bool xInRange = (x >= mins.x) && (x <= maxs.x);
	bool yInRange = (y >= mins.y) && (y <= maxs.y);
	if(xInRange && yInRange)
	{
		isInside = true;
	}

	return isInside;
}


//-------------------------------------------------------------------------------
bool AABB2D::IsPointInside(const Vector2& point) const
{
	bool isInside = IsPointInside(point.x, point.y);
	return isInside;
}


//-------------------------------------------------------------------------------
void AABB2D::operator+=(const Vector2& translation)
{
	Translate(translation);
}


//-------------------------------------------------------------------------------
void AABB2D::operator-=(const Vector2& translation)
{
	Translate(-1.0f * translation);
}


//-------------------------------------------------------------------------------
AABB2D AABB2D::operator+(const Vector2& translation) const
{
	AABB2D movedCopy = AABB2D(*this);
	movedCopy.Translate(translation);
	return movedCopy;
}


//-------------------------------------------------------------------------------
AABB2D AABB2D::operator-(const Vector2& translation) const
{
	AABB2D movedCopy = AABB2D(*this);
	movedCopy.Translate(-1.0f * translation);
	return movedCopy;
}


//-------------------------------------------------------------------------------
const AABB2D AABB2D::ZERO_TO_ONE = AABB2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
