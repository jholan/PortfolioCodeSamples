#include "Engine/Math/MathUtils.hpp"
#include <Math.h>
#include <cstdlib>
#include <limits>

#include "Engine/Core/EngineCommon.h"


//-------------------------------------------------------------------------------
const float FLOAT_EPSILON = std::numeric_limits<float>::epsilon();


//-------------------------------------------------------------------------------
bool FloatEquals(float lhs, float rhs)
{
	bool areEqual = false;

	float remainder = lhs - rhs;
	if (remainder <= FLOAT_EPSILON && remainder >= -FLOAT_EPSILON)
	{
		areEqual = true;
	}

	return areEqual;
}


//-------------------------------------------------------------------------------
bool FloatEquals(float lhs, float rhs, float epsilon)
{
	bool areEqual = false;

	float remainder = lhs - rhs;
	if (remainder <= epsilon && remainder >= -epsilon)
	{
		areEqual = true;
	}

	return areEqual;
}


//-------------------------------------------------------------------------------
float SQRT(float value)
{
	float squareRoot = sqrtf(value);
	return squareRoot;
}


//-------------------------------------------------------------------------------
float Abs(float value)
{
	float absValue = fabs(value);
	return absValue;
}


//-------------------------------------------------------------------------------
float Log2(float value)
{
	float lv = log2f(value);
	return lv;
}


//-------------------------------------------------------------------------------
float Log10(float value)
{
	float lv = log10f(value);
	return lv;
}


//-------------------------------------------------------------------------------
float LogE(float value)
{
	float lv = logf(value);
	return lv;
}


//-------------------------------------------------------------------------------
float Ceiling(float value)
{
	float c = ceilf(value);
	return c;
}


//-------------------------------------------------------------------------------
float Floor(float value)
{
	float f = floorf(value);
	return f;
}



//-------------------------------------------------------------------------------
Vector2 ConvertPolarToCartesian(float radius, float angleDegrees)
{
	Vector2 cartesianCoords = Vector2(radius * CosDegrees(angleDegrees), radius * SinDegrees(angleDegrees));
	return cartesianCoords;
}


//-------------------------------------------------------------------------------
Vector2 ConvertCartesianToPolar(const Vector2& cartesianXY)
{
	Vector2 polarCoords;

	polarCoords.x = cartesianXY.GetLength(); // R
	polarCoords.y = ATan2Degrees( cartesianXY.y, cartesianXY.x ); // theta

	return polarCoords;
}


//-------------------------------------------------------------------------------
Vector3 ConvertSphericalToCartesian(float radius, float polarAngleDegrees, float azimuthAngleDegrees)
{
	Vector3 cartesianCoords;

	cartesianCoords.x =  radius * CosDegrees(azimuthAngleDegrees) * CosDegrees(polarAngleDegrees);
	cartesianCoords.y =  radius * SinDegrees(azimuthAngleDegrees);
	cartesianCoords.z = -radius * CosDegrees(azimuthAngleDegrees) * SinDegrees(polarAngleDegrees);

	return cartesianCoords;
}


//-------------------------------------------------------------------------------
float ConvertDegreesToRadians(float angleDegrees)
{
	float angleRadians = angleDegrees * (PI / 180.0f);
	return angleRadians;
}


//-------------------------------------------------------------------------------
float ConvertRadiansToDegrees(float angleRadians)
{
	float angleDegrees = angleRadians * (180.0f / PI);
	return angleDegrees;
}


//-------------------------------------------------------------------------------
float CosRadians(float angle)
{
	float cosRadians = cosf(angle);
	return cosRadians;
}


//-------------------------------------------------------------------------------
float CosDegrees(float angleDegrees)
{
	float angleRadians = ConvertDegreesToRadians(angleDegrees);
	float cosDegrees = CosRadians(angleRadians);
	return cosDegrees;
}


//-------------------------------------------------------------------------------
float SinRadians(float angle)
{
	float sinRadians = sinf(angle);
	return sinRadians;
}


//-------------------------------------------------------------------------------
float SinDegrees(float angleDegrees)
{
	float angleRadians = ConvertDegreesToRadians(angleDegrees);
	float sinDegrees = SinRadians(angleRadians);
	return sinDegrees;
}


//-------------------------------------------------------------------------------
float TanRadians(float angle)
{
	float tanRadians = tanf(angle);
	return tanRadians;
}


//-------------------------------------------------------------------------------
float TanDegrees(float angleDegrees)
{
	float angleRadians = ConvertDegreesToRadians(angleDegrees);
	float tanDegrees = TanRadians(angleRadians);
	return tanDegrees;
}


//-------------------------------------------------------------------------------
float ATanDegrees(float value)
{
	float aTanRadians = atanf(value);
	float aTanDegrees = ConvertRadiansToDegrees(aTanRadians);
	return aTanDegrees;
}


//-------------------------------------------------------------------------------
float ATan2Degrees(float y, float x)
{
	float angleRadians = atan2f(y, x);
	float angleDegrees = ConvertRadiansToDegrees(angleRadians);
	return angleDegrees;
}


//-------------------------------------------------------------------------------
float ACosDegrees(float cosTheta)
{
	float angleRadians = acosf(cosTheta);
	float angleDegrees = ConvertRadiansToDegrees(angleRadians);
	return angleDegrees;
}


//-------------------------------------------------------------------------------
float GetDistance(const Vector2& a, const Vector2& b)
{
	float distanceSquared = GetDistanceSquared(a, b);
	float distance = SQRT(distanceSquared);
	return distance;
}


//-------------------------------------------------------------------------------
float GetDistanceSquared(const Vector2& a, const Vector2& b)
{
	Vector2 displacement = b - a;
	float distanceSquared = (displacement.x * displacement.x) + (displacement.y * displacement.y);
	return distanceSquared;
}

//-------------------------------------------------------------------------------
bool DoAABBsOverlap(const AABB2D& a, const AABB2D& b)
{
	Vector2 aHalfDimensions = a.GetDimensions() / 2.0f;
	Vector2 bHalfDimensions = b.GetDimensions() / 2.0f;
	Vector2 aCenter = a.GetCenter();
	Vector2 bCenter = b.GetCenter();
	AABB2D  scaledA = AABB2D(aCenter, aHalfDimensions + bHalfDimensions);

	bool areOverlapped = scaledA.IsPointInside(bCenter);
	return areOverlapped;
}


//-------------------------------------------------------------------------------
bool DoDiscsOverlap(const Disc2D& a, const Disc2D& b)
{
	bool areOverlapped = false;
	float distanceBetweenDiscs = GetDistance(a.center, b.center);
	float combinedRadii = a.radius + b.radius;
	if(distanceBetweenDiscs <= combinedRadii)
	{
		areOverlapped = true;
	}
	return areOverlapped;
}


//-------------------------------------------------------------------------------
bool DoDiscsOverlap(const Vector2& aCenter, float aRadius,
					const Vector2& bCenter, float bRadius)
{
	Disc2D a = Disc2D(aCenter, aRadius);
	Disc2D b = Disc2D(bCenter, bRadius);
	bool areOverlapped = DoDiscsOverlap(a, b);
	return areOverlapped;
}


//-------------------------------------------------------------------------------
bool DoRangesOverlap(const FloatRange& a, const FloatRange& b)
{
	bool areOverlapped = false;
	const FloatRange* leftRange = nullptr;
	const FloatRange* rightRange = nullptr;

	if (a.min <= b.min)
	{
		leftRange = &a;
		rightRange = &b;
	}
	else
	{
		leftRange = &b;
		rightRange = &a;
	}

	if ( (rightRange->min >= leftRange->min) && (rightRange->min <= leftRange->max) )
	{
		areOverlapped = true;
	}

	return areOverlapped;
}


//-------------------------------------------------------------------------------
bool DoRangesOverlap(const IntRange& a, const IntRange& b, bool inclusive)
{
	bool areOverlapped = false;
	const IntRange* leftRange = nullptr;
	const IntRange* rightRange = nullptr;

	if (a.min <= b.min)
	{
		leftRange = &a;
		rightRange = &b;
	}
	else
	{
		leftRange = &b;
		rightRange = &a;
	}

	
	if ( inclusive && (rightRange->min >= leftRange->min) && (rightRange->min <= leftRange->max) )
	{
		areOverlapped = true;
	}
	else if (!inclusive && (rightRange->min >= leftRange->min) && (rightRange->min < leftRange->max))
	{
		areOverlapped = true;
	}

	return areOverlapped;
}


//-------------------------------------------------------------------------------
float StringToFloat(const char* string, char** firstNonNumberChar)
{
	float result = (float)strtod(string, firstNonNumberChar);
	return result;
}


//-------------------------------------------------------------------------------
int StringToInt(const char* string, char** firstNonNumberChar)
{
	int result = (int)strtol(string, firstNonNumberChar, 10);
	return result;
}


//-------------------------------------------------------------------------------
float Min(float a, float b)
{
	float min = a;
	if(b < a)
	{
		min = b;
	}
	return min;
}



//-------------------------------------------------------------------------------
int Min(int a, int b)
{
	int min = a;
	if(b < a)
	{
		min = b;
	}
	return min;
}


//-------------------------------------------------------------------------------
Vector3 Min(const Vector3& a, const Vector3& b)
{
	Vector3 componentWiseMins;

	componentWiseMins.x = Min(a.x, b.x);
	componentWiseMins.y = Min(a.y, b.y);
	componentWiseMins.z = Min(a.z, b.z);


	return componentWiseMins;
}


//-------------------------------------------------------------------------------
float Max(float a, float b)
{
	float max = a;
	if(b > a)
	{
		max = b;
	}
	return max;
}


//-------------------------------------------------------------------------------
int Max(int a, int b)
{
	int max = a;
	if(b > a)
	{
		max = b;
	}
	return max;
}


//-------------------------------------------------------------------------------
Vector3 Max(const Vector3& a, const Vector3& b)
{
	Vector3 componentWiseMaxs;

	componentWiseMaxs.x = Max(a.x, b.x);
	componentWiseMaxs.y = Max(a.y, b.y);
	componentWiseMaxs.z = Max(a.z, b.z);


	return componentWiseMaxs;
}


//-------------------------------------------------------------------------------
void SeedRandomNumberGenerator(unsigned int seed)
{
	srand(seed);
}


//-------------------------------------------------------------------------------
int GetRandomIntLessThan( int exclusiveMax )
{
	int randomNumber = rand() % exclusiveMax;
	return randomNumber;
}


//-------------------------------------------------------------------------------
int GetRandomIntInRange( int min, int max )
{
	int range = (max - min) + 1;
	int randomNumber = (rand() % range) + min;
	return randomNumber;
}


//-------------------------------------------------------------------------------
float GetRandomFloatZeroToOne()
{
	float scaleFactor = 1.0f / (float)RAND_MAX;
	float randomNumber = rand() * scaleFactor;
	return randomNumber;
}


//-------------------------------------------------------------------------------
float GetRandomFloatInRange( float min, float max )
{
	float range = max - min;
	float randomNumber = (GetRandomFloatZeroToOne() * range) + min;
	return randomNumber;
}


//-------------------------------------------------------------------------------
bool CheckRandomChance( float chanceForSuccess )
{
	float roll = GetRandomFloatInRange(0.0f, 1.0f);
	bool succeded = false;
	if (roll <= chanceForSuccess)
	{
		succeded = true;
	}
	return succeded;
}


//-------------------------------------------------------------------------------
Vector3 GetRandomVector3()
{
	return Vector3(GetRandomFloatInRange(-1.0f, 1.0f), GetRandomFloatInRange(-1.0f, 1.0f), GetRandomFloatInRange(-1.0f, 1.0f)).GetNormalized();
}


//-------------------------------------------------------------------------------
Vector2 GetRandomVector2()
{
	return Vector2(GetRandomFloatInRange(-1.0f, 1.0f), GetRandomFloatInRange(-1.0f, 1.0f)).GetNormalized();
}


//-------------------------------------------------------------------------------
int RoundToNearestInt( float inValue )
{
	int intPart = (int)(inValue);
	float remainder = inValue - intPart;

	

	int nearest = intPart;
	if (remainder >= 0.5f)
	{
		nearest = intPart + 1;
	}
	else if (remainder < -0.5f)
	{
		nearest = intPart - 1;
	}

	return nearest;
}


//-------------------------------------------------------------------------------
int ClampInt( int inValue, int min, int max )
{
	int clampedValue = inValue;
	if (inValue < min)
	{
		clampedValue = min;
	}
	else if (inValue > max)
	{
		clampedValue = max;
	}
	return clampedValue;
}


//-------------------------------------------------------------------------------
float ClampFloat( float inValue, float min, float max )
{
	float clampedValue = inValue;
	if (inValue < min)
	{
		clampedValue = min;
	}
	else if (inValue > max)
	{
		clampedValue = max;
	}
	return clampedValue;
}


//-------------------------------------------------------------------------------
float ClampFloatZeroToOne( float inValue )
{
	float clampedValue = inValue;
	if (inValue < 0.0f)
	{
		clampedValue = 0.0f;
	}
	else if (inValue > 1.0f)
	{
		clampedValue = 1.0f;
	}
	return clampedValue;
}


//-------------------------------------------------------------------------------
float ClampFloatNegativeOneToOne( float inValue )
{
	float clampedValue = inValue;
	if (inValue < -1.0f)
	{
		clampedValue = -1.0f;
	}
	else if (inValue > 1.0f)
	{
		clampedValue = 1.0f;
	}
	return clampedValue;
}


//-------------------------------------------------------------------------------
float GetFractionInRange( float inValue, float rangeStart, float rangeEnd )
{
	float range = rangeEnd -rangeStart;
	if (range == 0.0f)
	{
		return 1.0f; // who knows?
	}

	float intoRange = inValue - rangeStart;
	float fractionIntoRange = intoRange / range;
	return fractionIntoRange;
}


//-------------------------------------------------------------------------------
float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	// Short circuit if we would divide by zero
	if (inStart == inEnd)
	{
		return (outStart + outEnd) * 0.5f;
	}

	// Else proceed as normal
	float inRange = inEnd - inStart;
	float outRange = outEnd - outStart;
	float inRelativeToStart = inValue - inStart;
	float fractionIntoRange = inRelativeToStart / inRange;
	float outRelativeToStart = fractionIntoRange * outRange;
	float outValue = outRelativeToStart + outStart;

	return outValue;
}


//-------------------------------------------------------------------------------
Vector2 RangeMapVector2(Vector2 inValue, Vector2 inStart, Vector2 inEnd, Vector2 outStart, Vector2 outEnd)
{
	Vector2 outValue;

	outValue.x = RangeMapFloat(inValue.x, inStart.x, inEnd.x, outStart.x, outEnd.x);
	outValue.y = RangeMapFloat(inValue.y, inStart.y, inEnd.y, outStart.y, outEnd.y);

	return outValue;
}


//-------------------------------------------------------------------------------
Vector3 RangeMapVector3(Vector3 inValue, Vector3 inStart, Vector3 inEnd, Vector3 outStart, Vector3 outEnd)
{
	Vector3 outValue;

	outValue.x = RangeMapFloat(inValue.x, inStart.x, inEnd.x, outStart.x, outEnd.x);
	outValue.y = RangeMapFloat(inValue.y, inStart.y, inEnd.y, outStart.y, outEnd.y);
	outValue.z = RangeMapFloat(inValue.z, inStart.z, inEnd.z, outStart.z, outEnd.z);

	return outValue;
}


//-------------------------------------------------------------------------------
Vector4 RangeMapVector4(Vector4 inValue, Vector4 inStart, Vector4 inEnd, Vector4 outStart, Vector4 outEnd)
{
	Vector4 outValue;

	outValue.x = RangeMapFloat(inValue.x, inStart.x, inEnd.x, outStart.x, outEnd.x);
	outValue.y = RangeMapFloat(inValue.y, inStart.y, inEnd.y, outStart.y, outEnd.y);
	outValue.z = RangeMapFloat(inValue.z, inStart.z, inEnd.z, outStart.z, outEnd.z);
	outValue.w = RangeMapFloat(inValue.w, inStart.w, inEnd.w, outStart.w, outEnd.w);

	return outValue;
}


//-------------------------------------------------------------------------------
float Interpolate( float start, float end, float fractionTowardEnd )
{
	float result;

	float ratio = fractionTowardEnd; //ClampFloatZeroToOne(fractionTowardEnd);
	result = start + ((end - start) * ratio);

	return result;
}


//-------------------------------------------------------------------------------
int Interpolate( int start,	int end, float fractionTowardEnd )
{
	if (fractionTowardEnd == 1.0f)
	{
		return end;
	}

	int range = end - start + 1; // include end
	int lerpedInt = start + (int)(range * fractionTowardEnd);
	//float fractionPerInt = 1.0f / range;
	//int currentIndex = (int)(fractionTowardEnd / fractionPerInt); 
	//int lerpedInt = start + currentIndex;
	return lerpedInt;
}


//-------------------------------------------------------------------------------
unsigned char Interpolate( unsigned char start,	unsigned char end, float fractionTowardEnd )
{
	int lerpedInt = Interpolate((int)start, (int)end, fractionTowardEnd);
	lerpedInt = ClampInt(lerpedInt, 0, 255);
	unsigned char lerpedChar = (unsigned char)lerpedInt;
	return lerpedChar;
}


//-------------------------------------------------------------------------------
const Vector2 Interpolate( const Vector2& start, const Vector2& end,	float fractionTowardEnd )
{
	Vector2 interpolatedVector = Vector2( Interpolate(start.x, end.x, fractionTowardEnd), Interpolate(start.y, end.y, fractionTowardEnd) );
	return interpolatedVector;
}


//-------------------------------------------------------------------------------
const Vector3 Interpolate( const Vector3&		start,	const Vector3&		end,	float fractionTowardEnd )
{
	Vector3 interpolatedVector = Vector3( 
		Interpolate(start.x, end.x, fractionTowardEnd),
		Interpolate(start.y, end.y, fractionTowardEnd),
		Interpolate(start.z, end.z, fractionTowardEnd));
	return interpolatedVector;
}


//-------------------------------------------------------------------------------
const IntVector2 Interpolate( const IntVector2&	start, const IntVector2& end, float fractionTowardEnd )
{
	IntVector2 lerpedVector = IntVector2( Interpolate(start.x, end.x, fractionTowardEnd), Interpolate(start.y, end.y, fractionTowardEnd) );
	return lerpedVector;
}


//-------------------------------------------------------------------------------
const FloatRange Interpolate( const FloatRange&	start, const FloatRange& end, float fractionTowardEnd )
{
	FloatRange lerpedRange = FloatRange( Interpolate(start.min, end.min, fractionTowardEnd), Interpolate(start.max, end.max, fractionTowardEnd) );
	return lerpedRange;
}


//-------------------------------------------------------------------------------
const IntRange Interpolate( const IntRange& start, const IntRange& end, float fractionTowardEnd )
{
	IntRange lerpedRange = IntRange( Interpolate(start.min, end.min, fractionTowardEnd), Interpolate(start.max, end.max, fractionTowardEnd) );
	return lerpedRange;
}


//-------------------------------------------------------------------------------
const AABB2D Interpolate( const AABB2D& start, const AABB2D& end, float fractionTowardEnd )
{
	AABB2D lerpedAABB = AABB2D( Interpolate(start.mins, end.mins, fractionTowardEnd), Interpolate(start.maxs, end.maxs, fractionTowardEnd) );
	return lerpedAABB;
}


//-------------------------------------------------------------------------------
const Disc2D Interpolate( const Disc2D& start, const Disc2D& end, float fractionTowardEnd )
{
	Disc2D lerpedDisc = Disc2D( Interpolate(start.center, end.center, fractionTowardEnd), Interpolate(start.radius, end.radius, fractionTowardEnd) );
	return lerpedDisc;
}


//-------------------------------------------------------------------------------
const RGBA Interpolate( const RGBA& start, const RGBA& end, float fractionTowardEnd )
{
	RGBA lerpedRGBA = RGBA( Interpolate(start.r, end.r, fractionTowardEnd),
							Interpolate(start.g, end.g, fractionTowardEnd),
							Interpolate(start.b, end.b, fractionTowardEnd),
							Interpolate(start.a, end.a, fractionTowardEnd));
	return lerpedRGBA;
}


//-------------------------------------------------------------------------------
bool AreBitsSet( unsigned char bitFlags8, unsigned char flagsToCheck )
{
	bool isSet = (bitFlags8 & flagsToCheck) == flagsToCheck;
	return isSet;
}


//-------------------------------------------------------------------------------
bool AreBitsSet(unsigned short bitFlags16, unsigned short flagsToCheck)
{
	bool isSet = (bitFlags16 & flagsToCheck) == flagsToCheck;
	return isSet;
}


//-------------------------------------------------------------------------------
bool AreBitsSet( unsigned int bitFlags32, unsigned int flagsToCheck )
{
	bool isSet = (bitFlags32 & flagsToCheck) == flagsToCheck;
	return isSet;
}


//-------------------------------------------------------------------------------
void SetBits( unsigned char& bitFlags8, unsigned char flagsToSet )
{
	bitFlags8 = bitFlags8 | flagsToSet;
}


//-------------------------------------------------------------------------------
void SetBits( unsigned short& bitFlags16, unsigned short flagsToSet )
{
	bitFlags16 = bitFlags16 | flagsToSet;
}


//-------------------------------------------------------------------------------
void SetBits( unsigned int& bitFlags32, unsigned int flagsToSet )
{
	bitFlags32 = bitFlags32 | flagsToSet;
}


//-------------------------------------------------------------------------------
void ClearBits( unsigned char& bitFlags8, unsigned char flagToClear )
{
	bitFlags8 = bitFlags8 & (~flagToClear);
}


//-------------------------------------------------------------------------------
void ClearBits( unsigned short& bitFlags16, unsigned short flagToClear )
{
	bitFlags16 = bitFlags16 & (~flagToClear);
}


//-------------------------------------------------------------------------------
void ClearBits( unsigned int& bitFlags32, unsigned int flagToClear )
{
	bitFlags32 = bitFlags32 & (~flagToClear);
}


//-----------------------------------------------------------------------------------------------
int GetIndexFromCoords(const IntVector2& coordinates, const IntVector2& dimensions)
{
	int index = coordinates.x + (dimensions.x * coordinates.y);
	return index;
}


//-----------------------------------------------------------------------------------------------
int GetIndexFromCoords(const IntVector3& coordinates, const IntVector3& dimensions)
{
	int index = coordinates.x + (dimensions.x * coordinates.y) + (dimensions.x * dimensions.y * coordinates.z);
	return index;
}


//-----------------------------------------------------------------------------------------------
IntVector2 GetCoordsFromIndex(int index, const IntVector2& dimensions)
{
	float fraction = (float)index / (float)dimensions.x;
	int intPart = (int)(fraction);
	int remaining = index - (intPart * dimensions.x);
	IntVector2 coords = IntVector2(remaining, intPart);
	return coords;
}


//-----------------------------------------------------------------------------------------------
IntVector3 GetCoordsFromIndex(int index, const IntVector3& dimensions)
{
	int indicesPerZIncrease = dimensions.x * dimensions.y;
	int zCoord = index / indicesPerZIncrease;

	IntVector2 xyCoords = (index - (indicesPerZIncrease * zCoord), IntVector2(dimensions.x, dimensions.y));
	
	IntVector3 xyzCoords = IntVector3(xyCoords.x, xyCoords.y, zCoord);
	return xyzCoords;
}


//-------------------------------------------------------------------------------
float GetAngularDisplacement( float startDegrees, float endDegrees )
{
	float angularDisp = endDegrees - startDegrees;
	
	// Detect worst case scenario and correct
	if (angularDisp >= 2000)
	{
		angularDisp = fmodf(angularDisp, 360.0f);
	}

	while(angularDisp > 180.0f)
	{
		angularDisp -= 360.0f;
	}
	while(angularDisp < -180.0f)
	{
		angularDisp += 360.0f;
	}

	return angularDisp;
}


//-------------------------------------------------------------------------------
float TurnToward( float currentDegrees, float goalDegrees, float maxTurnDegrees )
{
	float angularDisp = GetAngularDisplacement(currentDegrees, goalDegrees);
	
	float dispSign = 1.0f;
	if (angularDisp < 0)
	{
		dispSign = -1.0f;
	}

	float degreesToTurn = maxTurnDegrees;
	float absDisp = abs(angularDisp);
	if (absDisp < maxTurnDegrees)
	{
		degreesToTurn = absDisp;
	}
	float newDegrees = currentDegrees + (dispSign * degreesToTurn);

	return newDegrees;
}


//-------------------------------------------------------------------------------
float DotProduct( const Vector2& a, const Vector2& b )
{
	float dotProduct = (a.x * b.x) + (a.y * b.y);
	return dotProduct;
}



//-------------------------------------------------------------------------------
float DotProduct( const Vector3& a, const Vector3& b )
{
	float dotProduct = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	return dotProduct;
}


//-------------------------------------------------------------------------------
float DotProduct( const Vector4& a, const Vector4& b )
{
	float dotProduct = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
	return dotProduct;
}


//-------------------------------------------------------------------------------
Vector2 Reflect(const Vector2& vectorToReflect, const Vector2& normal)
{
	Vector2 reflectedVector = vectorToReflect;

	float vtrLenghtInNormalDirection = DotProduct(vectorToReflect, normal);
	Vector2 vtrNormalPart = normal * vtrLenghtInNormalDirection;

	reflectedVector -= (2.0f * vtrNormalPart);

	return reflectedVector;
}


//-------------------------------------------------------------------------------
const float GetProjectedMagnitude( const Vector2& vectorToProject, const Vector2& projectOnto )
{
	float projectedMagnitude = DotProduct(vectorToProject, projectOnto) / projectOnto.GetLengthSquared();
	return projectedMagnitude;
}


//-------------------------------------------------------------------------------
// Gets the projected vector in the “projectOnto” direction, whose magnitude is the projected length of “vectorToProject” in that direction.
const Vector2 GetProjectedVector( const Vector2& vectorToProject, const Vector2& projectOnto )
{
	Vector2 projectedVector = GetProjectedMagnitude(vectorToProject, projectOnto) * projectOnto;
	return projectedVector;
}


//-------------------------------------------------------------------------------
// Returns the vector’s representation/coordinates in (i,j) space (instead of its original x,y space)
const Vector2 GetTransformedIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ )
{
	float magnitudeInI = GetProjectedMagnitude(originalVector, newBasisI);
	float magnitudeInJ = GetProjectedMagnitude(originalVector, newBasisJ);

	Vector2 transformedVector = Vector2(magnitudeInI, magnitudeInJ);
	return transformedVector;
}


//-------------------------------------------------------------------------------
// Takes “vectorInBasis” in (i,j) space and returns the equivalent vector in [axis-aligned] (x,y) Cartesian space
const Vector2 GetTransformedOutOfBasis( const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ )
{
	Vector2 transformedVector = (vectorInBasis.x * oldBasisI) + (vectorInBasis.y * oldBasisJ);
	return transformedVector;
}


//-------------------------------------------------------------------------------
// Decomposes “originalVector” into two component vectors, which add up to the original:
//   “vectorAlongI” is the vector portion in the “newBasisI” direction, and
//   “vectorAlongJ” is the vector portion in the “newBasisJ” direction.
void DecomposeVectorIntoBasis( const Vector2& originalVector,
							   const Vector2& newBasisI, const Vector2& newBasisJ,
							   Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ )
{
	out_vectorAlongI = GetProjectedVector(originalVector, newBasisI);
	out_vectorAlongJ = GetProjectedVector(originalVector, newBasisJ);
}


//-------------------------------------------------------------------------------
Vector3 CrossProduct(const Vector3& from, const Vector3& to)
{
	Vector3 cross;

	cross.x = (from.y * to.z) - (from.z * to.y);
	cross.y = (from.z * to.x) - (from.x * to.z);
	cross.z = (from.x * to.y) - (from.y * to.x);

	return cross;
}


//-------------------------------------------------------------------------------
float SmoothStart2( float t )
{
	float tOut = t * t;
	return tOut;
}


//-------------------------------------------------------------------------------
float SmoothStart3( float t )
{
	float tOut = t * t * t;
	return tOut;
}


//-------------------------------------------------------------------------------
float SmoothStart4( float t )
{
	float tOut = t * t * t * t;
	return tOut;
}


//-------------------------------------------------------------------------------
float SmoothStop2( float t ) 
{
	float tOut = 1.0f - ((1.0f - t) * (1.0f - t));
	return tOut;
}


//-------------------------------------------------------------------------------
float SmoothStop3( float t ) 
{
	float tOut = 1.0f - ((1.0f - t) * (1.0f - t) * (1.0f - t));
	return tOut;
}


//-------------------------------------------------------------------------------
float SmoothStop4( float t ) 
{
	float tOut = 1.0f - ((1.0f - t) * (1.0f - t) * (1.0f - t) * (1.0f - t));
	return tOut;
}


//-------------------------------------------------------------------------------
float SmoothStep3( float t ) 
{
	float tStart = SmoothStart3(t);
	float tStop = SmoothStop3(t);
	float tBlended = ((1.0f - t) * tStart) + (t * tStop);

	return tBlended;
}


//-------------------------------------------------------------------------------
std::vector<Vector3> GetPointsForProjection(const Matrix4& projection)
{
	std::vector<Vector3> points;

	// The NDC cube
	Vector4 frustumPoints[8] = {
		Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
		Vector4(1.0f, -1.0f, -1.0f, 1.0f),
		Vector4(1.0f, 1.0f, -1.0f, 1.0f),
		Vector4(-1.0f, 1.0f, -1.0f, 1.0f),

		Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
		Vector4(1.0f, -1.0f, 1.0f, 1.0f),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
	};


	Matrix4 inverseProjection = projection;
	inverseProjection.Invert();


	for (int i = 0; i < 8; ++i)
	{
		// Move the frustum points out into pre projection space, likely camera space
		frustumPoints[i] = inverseProjection * frustumPoints[i];
		frustumPoints[i] /= frustumPoints[i].w; // Perspective devide

		points.push_back(Vector3(frustumPoints[i].XYZ()));
	}

	return points;
}


//-------------------------------------------------------------------------------
Vector3	GetFrustumCenter(const std::vector<Vector3>& frustumPoints)
{
	GUARANTEE_OR_DIE(frustumPoints.size() == 8, "The numbert of frustum points should be 8");

	Vector3 frustumCenter = Vector3::ZEROS;

	for(int i = 0; i < 8; i++)
	{
		frustumCenter += frustumPoints[i];
	}
	frustumCenter /= 8.0f;


	return frustumCenter;
}


//-------------------------------------------------------------------------------
float GetFrustumEncompassingRadius(const std::vector<Vector3>& frustumPoints)
{
	//float r = (frustumPoints[3] - frustumPoints[5]).GetLength() * 0.5f;
	//return r;

	float farHLen = (frustumPoints[7] - frustumPoints[6]).GetLength();
	float farVLen = (frustumPoints[7] - frustumPoints[4]).GetLength();
	float dLen = (frustumPoints[7] - frustumPoints[3]).GetLength();
	float rad = (farHLen * 0.5f) + (farVLen * 0.5f) + (dLen * 0.5f);
	return rad * 0.5f;

	// Get the half distance between two of the most different frustum points
	Vector3 difference = frustumPoints[5] - frustumPoints[3]; 
	float length = difference.GetLength();
	float radius = length * 0.5f;
	return radius;
}


//-------------------------------------------------------------------------------
bool Quadratic( float a, float b, float c, float& out_root1, float& out_root2 )
{
	bool rootsExist = false;

	float descriminant = (b * b) - (4 * a * c);
	if ( (descriminant >= 0) && (a != 0.0f) )
	{
		rootsExist = true;

		float SQRT_descriminant = SQRT(descriminant);
		float one_DIV_2a = 1.0f / (2.0f * a);
		out_root1 = (-b + SQRT_descriminant) * one_DIV_2a;
		out_root2 = (-b - SQRT_descriminant) * one_DIV_2a;
	}

	return rootsExist;
}


//-------------------------------------------------------------------------------
Vector2 Trajectory_GetPositionAtTime( float gravity, float launchSpeed, float launchAngleDegrees, float time )
{
	Vector2 position;

	position.x = launchSpeed * CosDegrees(launchAngleDegrees) * time;
	position.y = ((-0.5f) * gravity * (time * time)) + (launchSpeed * SinDegrees(launchAngleDegrees) * time);

	return position;
}




float Trajectory_GetMinimumLaunchSpeed( float gravity, float distance )
{
	float gdi = SQRT(gravity * distance);
	return gdi;
}



bool Trajectory_GetLaunchAngles( float& out_angle1, float& out_angle2, float gravity, float launchSpeed, float distance, float height)
{
	bool hasAngles = false;

	float launchSpeedSquared = launchSpeed * launchSpeed;
	float distanceSquared = distance * distance;
	float descriminant = (launchSpeedSquared * launchSpeedSquared) - (gravity * ((gravity * distanceSquared) + (2 * height * launchSpeedSquared)));
	if ( (descriminant >= 0.0f) && ((gravity * distance) != 0.0f) )
	{
		float SQRT_descriminant = SQRT(descriminant);
		float one_DIV_gravityXdistance = 1.0f / (gravity * distance);
		out_angle1 = ConvertRadiansToDegrees((float)atan( (launchSpeedSquared + SQRT_descriminant) * one_DIV_gravityXdistance ));
		out_angle2 = ConvertRadiansToDegrees((float)atan( (launchSpeedSquared - SQRT_descriminant) * one_DIV_gravityXdistance ));
		hasAngles = true;
	}

	return hasAngles;
}



float Trajectory_GetMaxHeight( float gravity, float launchSpeed, float distance )
{
	float thetaRad = atan( (launchSpeed * launchSpeed) / (gravity * distance) );
	float thetaDeg = ConvertRadiansToDegrees(thetaRad);
	float time = distance / (launchSpeed * CosDegrees(thetaDeg));
	float height = ( -0.5f * gravity * (time * time) ) + ( launchSpeed * CosDegrees(thetaDeg) * time );

	return height;
}



Vector2 Trajectory_GetLaunchVelocity( float gravity, float apexHeight, float distance, float height )
{
	float timeToApex = SQRT( (2.0f * apexHeight) / gravity );
	float fallHeight = apexHeight - height;
	float fallingBodyHeight = SQRT( (2.0f * fallHeight) / gravity );
	float totalTime = timeToApex + fallingBodyHeight;

	Vector2 velocity;
	velocity.x = distance / totalTime;
	velocity.y = gravity * timeToApex;
	return velocity;
}