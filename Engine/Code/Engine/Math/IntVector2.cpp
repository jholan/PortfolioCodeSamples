#include "Engine/Math/IntVector2.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
IntVector2::IntVector2(const IntVector2& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
{
}


//-----------------------------------------------------------------------------------------------
IntVector2::IntVector2(int initialX, int initialY)
	: x(initialX)
	, y(initialY)
{
}


//-----------------------------------------------------------------------------------------------
IntVector2::IntVector2(const Vector2& vec)
	: x((int)vec.x)
	, y((int)vec.y)
{
}


//-----------------------------------------------------------------------------------------------
void IntVector2::SetFromText(const char* text)
{
	// If there is not comma we just dont set from text as it is not valid.
	const char* comma = FindFirstCharacterInString(text, ",");
	if ((comma != nullptr))
	{
		x = StringToInt(text);
		y = StringToInt(++comma);
	}
}

									
//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator+(const IntVector2& vecToAdd) const
{
	return IntVector2(x + vecToAdd.x, y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator-(const IntVector2& vecToSubtract) const
{
	return IntVector2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator*(int uniformScale) const
{
	return IntVector2(x * uniformScale, y * uniformScale);
}


//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator/(int inverseScale) const
{
	return IntVector2(x / inverseScale, y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator+=(const IntVector2& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator-=(const IntVector2& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator*=(const int uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator/=(const int uniformDivisor)
{
	int oneOverUniformDivisor = 1 / uniformDivisor;
	x *= oneOverUniformDivisor;
	y *= oneOverUniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator=(const  IntVector2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
bool IntVector2::operator==(const IntVector2& compare) const
{
	bool result = false;

	if((x == compare.x) && (y == compare.y))
	{
		result = true;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
bool IntVector2::operator!=(const IntVector2& compare) const
{
	bool result = (*this == compare);

	return !result;
}


//-----------------------------------------------------------------------------------------------
const IntVector2 operator*(int uniformScale, const IntVector2& vecToScale)
{
	return IntVector2(vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}
