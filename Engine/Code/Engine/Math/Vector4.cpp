#include "Engine/Math/Vector4.hpp"

#include <cmath>

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector3.hpp"



//-----------------------------------------------------------------------------------------------
Vector4::Vector4(const Vector4& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
	, w(copy.w)
{
}


//-----------------------------------------------------------------------------------------------
Vector4::Vector4(float initialX, float initialY, float initialZ, float initialW)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
}


//-----------------------------------------------------------------------------------------------
Vector4::Vector4(const Vector3& initialXYZ, float initialW)
	: x(initialXYZ.x)
	, y(initialXYZ.y)
	, z(initialXYZ.z)
	, w(initialW)
{
}


//-----------------------------------------------------------------------------------------------
void Vector4::SetFromText(const char* text)
{
	UNUSED(text);
	UNIMPLEMENTED();
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator+ (const Vector4& vecToAdd) const
{
	return Vector4(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w);
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator-() const
{
	return Vector4(-x, -y, -z, -w);
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator-(const Vector4& vecToSubtract) const
{
	return Vector4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator*(float uniformScale) const
{
	return Vector4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale);
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator/(float inverseScale) const
{
	float oneOverInverseScale = 1.0f / inverseScale;
	return Vector4(x * oneOverInverseScale, y * oneOverInverseScale, z * oneOverInverseScale, w * oneOverInverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator+=(const Vector4& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator-=(const Vector4& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator/=(const float uniformDivisor)
{
	float oneOverUniformDivisor = 1.0f / uniformDivisor;
	x *= oneOverUniformDivisor;
	y *= oneOverUniformDivisor;
	z *= oneOverUniformDivisor;
	w *= oneOverUniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator=(const Vector4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}


//-----------------------------------------------------------------------------------------------
const Vector4 operator*(float uniformScale, const Vector4& vecToScale)
{
	return Vector4(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale, vecToScale.w * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vector4::operator==(const Vector4& compare) const
{
	bool result = false;

	if((x == compare.x) && (y == compare.y) && (z == compare.z) && (w == compare.w))
	{
		result = true;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
bool Vector4::operator!=(const Vector4& compare) const
{
	bool result = (*this == compare);

	return !result;
}


//-----------------
float Vector4::GetLength() const
{
	float length = SQRT((x * x) + (y * y) + (z * z));
	return length;
}


//--------------------
float Vector4::GetLengthSquared() const
{
	float lengthSquared = (x * x) + (y * y) + (z * z);
	return lengthSquared;
}


//------------------------
float Vector4::NormalizeAndGetLength()
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
		w /= length;
	}
	return length;
}


//-------------------------
Vector4 Vector4::GetNormalized() const
{
	Vector4 normalizedVector = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
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
Vector4 Vector4::GetHadamard(const Vector4 & scaleFactors) const
{
	Vector4 hadamardProduct = *this;
	hadamardProduct.x *= scaleFactors.x;
	hadamardProduct.y *= scaleFactors.y;
	hadamardProduct.z *= scaleFactors.z;
	hadamardProduct.w *= scaleFactors.w;
	return hadamardProduct;
}


//-------------------------
Vector3 Vector4::XYZ() const
{
	return Vector3(x, y, z);
}


//-------------------------
const Vector4 Vector4::ZEROS = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::ONES  = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

const Vector4 Vector4::X_AXIS = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::Y_AXIS = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::Z_AXIS = Vector4(0.0f, 0.0f, 1.0f, 0.0f);

const Vector4 Vector4::RIGHT   = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::UP      = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::FORWARD = Vector4(0.0f, 0.0f, 1.0f, 0.0f);

const Vector4 Vector4::I = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::J = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::K = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
