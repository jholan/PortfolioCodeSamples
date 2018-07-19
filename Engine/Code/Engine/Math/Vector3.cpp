#include "Engine/Math/Vector3.hpp"

#include <cmath>

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector3.hpp"



//-----------------------------------------------------------------------------------------------
Vector3::Vector3(const Vector3& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{
}


//-----------------------------------------------------------------------------------------------
Vector3::Vector3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}


//-----------------------------------------------------------------------------------------------
Vector3::Vector3(const Vector2& initialXY, float initialZ)
	: x(initialXY.x)
	, y(initialXY.y)
	, z(initialZ)
{
}



//-----------------------------------------------------------------------------------------------
Vector3::Vector3(const IntVector3& ivec)
	: x((float)ivec.x)
	, y((float)ivec.y)
	, z((float)ivec.z)
{
}


//-----------------------------------------------------------------------------------------------
void Vector3::SetFromText(const char* text)
{
	// If there is not comma we just dont set from text as it is not valid.
	const char* start = FindFirstCharacterInString(text, "0123456789-+"); 
	const char* comma = FindFirstCharacterInString(text, ",");
	if ((comma != nullptr))
	{
		const char* comma2 = FindFirstCharacterInString(comma + 1, ",");
		if (comma2 != nullptr)
		{
			x = StringToFloat(start);
			y = StringToFloat(++comma);
			z = StringToFloat(++comma2);
		}
	}
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator+ (const Vector3& vecToAdd) const
{
	return Vector3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator-(const Vector3& vecToSubtract) const
{
	return Vector3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator*(float uniformScale) const
{
	return Vector3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator/(float inverseScale) const
{
	float oneOverInverseScale = 1.0f / inverseScale;
	return Vector3(x * oneOverInverseScale, y * oneOverInverseScale, z * oneOverInverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator+=(const Vector3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator-=(const Vector3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator/=(const float uniformDivisor)
{
	float oneOverUniformDivisor = 1.0f / uniformDivisor;
	x *= oneOverUniformDivisor;
	y *= oneOverUniformDivisor;
	z *= oneOverUniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator=(const Vector3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vector3 operator*(float uniformScale, const Vector3& vecToScale)
{
	return Vector3(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vector3::operator==(const Vector3& compare) const
{
	bool result = false;

	if((x == compare.x) && (y == compare.y) && (z == compare.z))
	{
		result = true;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
bool Vector3::operator!=(const Vector3& compare) const
{
	bool result = (*this == compare);

	return !result;
}


//-----------------
float Vector3::GetLength() const
{
	float length = SQRT((x * x) + (y * y) + (z * z));
	return length;
}


//--------------------
float Vector3::GetLengthSquared() const
{
	float lengthSquared = (x * x) + (y * y) + (z * z);
	return lengthSquared;
}


//------------------------
float Vector3::NormalizeAndGetLength()
{
	float length = GetLength();
	if (length == 0.0f)
	{
		// Do nothing
	}
	else
	{
		// Else actually normalize the vector.
		x /= length;
		y /= length;
		z /= length;
	}
	return length;
}


//-------------------------
Vector3 Vector3::GetNormalized() const
{
	Vector3 normalizedVector = Vector3(0.0f, 0.0f, 0.0f);
	float length = GetLength();
	if (length == 0.0f)
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
Vector3 Vector3::GetHadamard(const Vector3 & scaleFactors) const
{
	Vector3 hadamardProduct = *this;
	hadamardProduct.x *= scaleFactors.x;
	hadamardProduct.y *= scaleFactors.y;
	hadamardProduct.z *= scaleFactors.z;
	return hadamardProduct;
}


//-------------------------
Vector3 Vector3::XZY() const
{
	Vector3 swizzled = Vector3(x, z, y);
	return swizzled;
}



Vector2 Vector3::XZ() const
{
	Vector2 swizzled = Vector2(x, z);
	return swizzled;
}


//-------------------------
const Vector3 Vector3::ZEROS = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONES  = Vector3(1.0f, 1.0f, 1.0f);

const Vector3 Vector3::X_AXIS = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Y_AXIS = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::Z_AXIS = Vector3(0.0f, 0.0f, 1.0f);

const Vector3 Vector3::RIGHT   = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UP      = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::FORWARD = Vector3(0.0f, 0.0f, 1.0f);

const Vector3 Vector3::I = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::J = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::K = Vector3(0.0f, 0.0f, 1.0f);
