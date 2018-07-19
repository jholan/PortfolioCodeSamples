#include "Engine/Math/Vector2.hpp"
#include <cmath>

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/IntVector2.hpp"



//-----------------------------------------------------------------------------------------------
Vector2::Vector2(const Vector2& copy)
	: x(copy.x)
	, y(copy.y)
{
}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2(float initialX, float initialY)
	: x(initialX)
	, y(initialY)
{
}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2(const IntVector2& ivec)
	: x((float)ivec.x)
	, y((float)ivec.y)
{
}


//-----------------------------------------------------------------------------------------------
void Vector2::SetFromText(const char* text)
{
	// If there is not comma we just dont set from text as it is not valid.
	const char* comma = FindFirstCharacterInString(text, ",");
	if ((comma != nullptr))
	{
		x = StringToFloat(text);
		y = StringToFloat(++comma);
	}
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator+ (const Vector2& vecToAdd) const
{
	return Vector2(x + vecToAdd.x, y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator-(const Vector2& vecToSubtract) const
{
	return Vector2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator*(float uniformScale) const
{
	return Vector2(x * uniformScale, y * uniformScale);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/(float inverseScale) const
{
	float oneOverInverseScale = 1.0f / inverseScale;
	return Vector2(x * oneOverInverseScale, y * oneOverInverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator+=(const Vector2& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator-=(const Vector2& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator/=(const float uniformDivisor)
{
	float oneOverUniformDivisor = 1.0f / uniformDivisor;
	x *= oneOverUniformDivisor;
	y *= oneOverUniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator=(const Vector2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vector2 operator*(float uniformScale, const Vector2& vecToScale)
{
	return Vector2(vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator==(const Vector2& compare) const
{
	bool result = false;

	if((x == compare.x) && (y == compare.y))
	{
		result = true;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator!=(const Vector2& compare) const
{
	bool result = (*this == compare);

	return !result;
}


//-----------------
float Vector2::GetLength() const
{
	float length = SQRT((x * x) + (y * y));
	return length;
}


//--------------------
float Vector2::GetLengthSquared() const
{
	float lengthSquared = (x * x) + (y * y);
	return lengthSquared;
}


//------------------------
float Vector2::NormalizeAndGetLength()
{
	float length = GetLength();
	if (length == 0.0f)
	{
		// Do nothing
	}
	else
	{
		// Else actually normalize the vector.
		this->x /= length;
		this->y /= length;
	}
	return length;
}


//-------------------------
Vector2 Vector2::GetNormalized() const
{
	Vector2 normalizedVector = Vector2(0.0f, 0.0f);
	float length = GetLength();
	if (length <= 0.001f)
	{
		// Do nothing 
	}
	else
	{
		// Else actually normalize the vector.
		normalizedVector = *this / length;
	}
	
	return normalizedVector;
}


//-------------------------
Vector2 Vector2::GetHadamard(const Vector2 & scaleFactors) const
{
	Vector2 hadamardProduct = *this;
	hadamardProduct.x *= scaleFactors.x;
	hadamardProduct.y *= scaleFactors.y;
	return hadamardProduct;
}


//---------------------------
float Vector2::GetOrientationDegrees() const
{
	float orientationRadians = atan2f(this->y, this->x);
	float orientationDegrees = ConvertRadiansToDegrees(orientationRadians);
	return orientationDegrees;
}


//----------------------------
Vector2 Vector2::MakeDirectionAtDegrees(float degrees)
{
	Vector2 direction = Vector2(CosDegrees(degrees), SinDegrees(degrees));
	return direction;
}


//----------------------------
const Vector2 Vector2::ZEROS	= Vector2(0.0f, 0.0f);
const Vector2 Vector2::ONES		= Vector2(1.0f, 1.0f);

const Vector2 Vector2::X_AXIS	= Vector2(1.0f, 0.0f);
const Vector2 Vector2::Y_AXIS	= Vector2(0.0f, 1.0f);

const Vector2 Vector2::RIGHT	= Vector2(1.0f, 0.0f);
const Vector2 Vector2::FORWARD	= Vector2(0.0f, 1.0f);
