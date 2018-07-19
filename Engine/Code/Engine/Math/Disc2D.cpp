#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Disc2D.hpp"



//-----------------------------------------------------------------------------------------------
Disc2D::Disc2D(const Disc2D& copyFrom)
	: center(copyFrom.center)
	, radius(copyFrom.radius)
{
}


//-----------------------------------------------------------------------------------------------
Disc2D::Disc2D(float initialX, float initialY, float initialRadius)
	: center(initialX, initialY)
	, radius(initialRadius)
{
}


//-----------------------------------------------------------------------------------------------
Disc2D::Disc2D(const Vector2& initialCenter, float initialRadius)
	: center(initialCenter)
	, radius(initialRadius)
{
}


//-----------------------------------------------------------------------------------------------
void Disc2D::StretchToIncludePoint(float x, float y)
{
	Vector2 point = Vector2(x, y);
	StretchToIncludePoint(point);
}


//-----------------------------------------------------------------------------------------------
void Disc2D::StretchToIncludePoint(const Vector2& point)
{
	float distanceToPoint = GetDistance(center, point);
	if(distanceToPoint > radius)
	{
		radius = distanceToPoint;
	}
}


//-----------------------------------------------------------------------------------------------
void Disc2D::AddPaddingToRadius(float paddingRadius)
{
	radius += paddingRadius;
}


//-----------------------------------------------------------------------------------------------
void Disc2D::Translate(const Vector2& translation)
{
	center += translation;
}


//-----------------------------------------------------------------------------------------------
void Disc2D::Translate(float translationX, float translationY)
{
	Vector2 translationVector = Vector2(translationX, translationY);
	Translate(translationVector);
}


//-----------------------------------------------------------------------------------------------
bool Disc2D::IsPointInside(float x, float y) const
{
	Vector2 point = Vector2(x, y);
	bool isInside = IsPointInside(point);
	return isInside;
}


//-----------------------------------------------------------------------------------------------
bool Disc2D::IsPointInside(const Vector2& point) const
{
	bool isInside = false;
	float distanceToPoint = GetDistance(center, point);
	if(distanceToPoint <= radius)
	{
		isInside = true;
	}
	return isInside;
}


//-----------------------------------------------------------------------------------------------
void Disc2D::operator+=(const Vector2& translation)
{
	Translate(translation);
}


//-----------------------------------------------------------------------------------------------
void Disc2D::operator-=(const Vector2& translation)
{
	Translate(-1.0f * translation);
}


//-----------------------------------------------------------------------------------------------
Disc2D Disc2D::operator+(const Vector2& translation) const
{
	Disc2D movedCopy = Disc2D(*this);
	movedCopy += translation;
	return movedCopy;
}


//-----------------------------------------------------------------------------------------------
Disc2D Disc2D::operator-(const Vector2& antiTranslation) const
{
	Disc2D movedCopy = Disc2D(*this);
	movedCopy -= antiTranslation;
	return movedCopy;
}