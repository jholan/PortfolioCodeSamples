#include "Engine/Math/IntVector3.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
IntVector3::IntVector3(const IntVector3& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
	, z(copyFrom.z)
{
}


//-----------------------------------------------------------------------------------------------
IntVector3::IntVector3(int initialX, int initialY, int initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}


//-----------------------------------------------------------------------------------------------
void IntVector3::SetFromText(const char* text)
{
	UNUSED(text);
	UNIMPLEMENTED();
}


//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator+(const IntVector3& vecToAdd) const
{
	return IntVector3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator-(const IntVector3& vecToSubtract) const
{
	return IntVector3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator*(int uniformScale) const
{
	return IntVector3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator/(int inverseScale) const
{
	int oneOverInverseScale = 1 / inverseScale;
	return IntVector3(x * oneOverInverseScale, y * oneOverInverseScale, z * oneOverInverseScale);
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator+=(const IntVector3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator-=(const IntVector3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator*=(const int uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator/=(const int uniformDivisor)
{
	int oneOverUniformDivisor = 1 / uniformDivisor;
	x *= oneOverUniformDivisor;
	y *= oneOverUniformDivisor;
	z *= oneOverUniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator=(const  IntVector3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
bool IntVector3::operator==(const IntVector3& compare) const
{
	bool result = false;

	if((x == compare.x) && (y == compare.y) && (z == compare.z))
	{
		result = true;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
bool IntVector3::operator!=(const IntVector3& compare) const
{
	bool result = (*this == compare);

	return !result;
}


//-----------------------------------------------------------------------------------------------
const IntVector3 operator*(int uniformScale, const IntVector3& vecToScale)
{
	return IntVector3(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale);
}
