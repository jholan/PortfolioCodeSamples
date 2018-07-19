#pragma once

#include <vector>

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Math/Disc2D.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Matrix4.hpp"


#define PI 3.1415926535897932384626433832795f

extern const float FLOAT_EPSILON;

bool  FloatEquals(float lhs, float rhs);
bool  FloatEquals(float lhs, float rhs, float epsilon);

float SQRT(float value);
float Abs(float value);

float Log2(float value);
float Log10(float value);
float LogE(float value);

float Ceiling(float value);
float Floor(float value);

Vector2 ConvertPolarToCartesian(float radius, float angleDegrees);
Vector2 ConvertCartesianToPolar(const Vector2& cartesianXY); //(R, theta)

Vector3 ConvertSphericalToCartesian(float radius, float polarAngleDegrees, float azimuthAngleDegrees); // polar == xy rot, azimuth == height rot

float ConvertDegreesToRadians(float angleDegrees);
float ConvertRadiansToDegrees(float angleRadians);

float CosRadians(float angle);
float CosDegrees(float angle);

float SinRadians(float angle);
float SinDegrees(float angle);

float TanRadians(float angle);
float TanDegrees(float angle);

float ATanDegrees(float value);
float ATan2Degrees(float y, float x);
float ACosDegrees(float cosTheta);


float GetDistance(const Vector2& a, const Vector2& b);
float GetDistanceSquared(const Vector2& a, const Vector2& b);


bool DoAABBsOverlap(const AABB2D& a, const AABB2D& b);
bool DoDiscsOverlap(const Disc2D& a, const Disc2D& b);
bool DoDiscsOverlap(const Vector2& aCenter, float aRadius,
					const Vector2& bCenter, float bRadius);
bool DoRangesOverlap(const FloatRange& a, const FloatRange& b);
bool DoRangesOverlap(const IntRange& a, const IntRange& b, bool inclusive = true);


float StringToFloat(const char* string, char** firstNonNumberChar = nullptr);
int   StringToInt(const char* string, char** firstNonNumberChar = nullptr);


float	Min(float a, float b);
int		Min(int   a, int   b);
Vector3 Min(const Vector3& a, const Vector3& b);

float	Max(float a, float b);
int		Max(int   a, int   b);
Vector3 Max(const Vector3& a, const Vector3& b);


void  SeedRandomNumberGenerator(unsigned int seed);
int   GetRandomIntLessThan( int exclusiveMax );			// Gives integers in [0,max-1]
int   GetRandomIntInRange( int min, int max );			// Gives integers in [min,max]
float GetRandomFloatZeroToOne();						// Gives floats in [0.0f,1.0f]
float GetRandomFloatInRange( float min, float max );	// Gives floats in [min,max]
bool  CheckRandomChance( float chanceForSuccess );		// If 0.27f passed, returns true 27% of the time
Vector3 GetRandomVector3();
Vector2 GetRandomVector2();


int RoundToNearestInt( float inValue );					// 0.5 rounds up to 1; -0.5 rounds up to 0
int   ClampInt( int inValue, int min, int max );
float ClampFloat( float inValue, float min, float max );
float ClampFloatZeroToOne( float inValue );
float ClampFloatNegativeOneToOne( float inValue );


float GetFractionInRange( float inValue, float rangeStart, float rangeEnd );
float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd);
Vector2 RangeMapVector2(Vector2 inValue, Vector2 inStart, Vector2 inEnd, Vector2 outStart, Vector2 outEnd);
Vector3 RangeMapVector3(Vector3 inValue, Vector3 inStart, Vector3 inEnd, Vector3 outStart, Vector3 outEnd);
Vector4 RangeMapVector4(Vector4 inValue, Vector4 inStart, Vector4 inEnd, Vector4 outStart, Vector4 outEnd);


float				Interpolate( float				start,	float				end,	float fractionTowardEnd );
int					Interpolate( int				start,	int					end,	float fractionTowardEnd );
unsigned char		Interpolate( unsigned char		start,	unsigned char		end,	float fractionTowardEnd );
const Vector2		Interpolate( const Vector2&		start,	const Vector2&		end,	float fractionTowardEnd );
const Vector3		Interpolate( const Vector3&		start,	const Vector3&		end,	float fractionTowardEnd );
const IntVector2	Interpolate( const IntVector2&	start,	const IntVector2&	end,	float fractionTowardEnd );
const FloatRange	Interpolate( const FloatRange&	start,	const FloatRange&	end,	float fractionTowardEnd );
const IntRange		Interpolate( const IntRange&	start,	const IntRange&		end,	float fractionTowardEnd );
const AABB2D		Interpolate( const AABB2D&		start,	const AABB2D&		end,	float fractionTowardEnd );
const Disc2D		Interpolate( const Disc2D&		start,	const Disc2D&		end,	float fractionTowardEnd );
const RGBA			Interpolate( const RGBA&		start,	const RGBA&			end,	float fractionTowardEnd );


bool AreBitsSet( unsigned char bitFlags8, unsigned char flagsToCheck );
bool AreBitsSet(unsigned short bitFlags16, unsigned short flagsToCheck);
bool AreBitsSet( unsigned int bitFlags32, unsigned int flagsToCheck );
void SetBits( unsigned char& bitFlags8, unsigned char flagsToSet );
void SetBits( unsigned short& bitFlags16, unsigned short flagsToSet );
void SetBits( unsigned int& bitFlags32, unsigned int flagsToSet );
void ClearBits( unsigned char& bitFlags8, unsigned char flagToClear );
void ClearBits( unsigned short& bitFlags8, unsigned short flagToClear );
void ClearBits( unsigned int& bitFlags32, unsigned int flagToClear );


int GetIndexFromCoords(const IntVector2& coordinates, const IntVector2& dimensions);
int GetIndexFromCoords(const IntVector3& coordinates, const IntVector3& dimensions);
IntVector2 GetCoordsFromIndex(int index, const IntVector2& dimensions);
IntVector3 GetCoordsFromIndex(int index, const IntVector3& dimensions);


float GetAngularDisplacement( float startDegrees, float endDegrees );
float TurnToward( float currentDegrees, float goalDegrees, float maxTurnDegrees );


float DotProduct( const Vector2& a, const Vector2& b );
float DotProduct( const Vector3& a, const Vector3& b );
float DotProduct( const Vector4& a, const Vector4& b );

Vector2 Reflect(const Vector2& vectorToReflect, const Vector2& normal); // Both vectors must be in the same space. Normal should be normalized
const float GetProjectedMagnitude( const Vector2& vectorToProject, const Vector2& projectOnto );
const Vector2 GetProjectedVector( const Vector2& vectorToProject, const Vector2& projectOnto );
const Vector2 GetTransformedIntoBasis( const Vector2& originalVector,
									   const Vector2& newBasisI, const Vector2& newBasisJ );
const Vector2 GetTransformedOutOfBasis( const Vector2& vectorInBasis,
										const Vector2& oldBasisI, const Vector2& oldBasisJ );
void DecomposeVectorIntoBasis( const Vector2& originalVector,
							   const Vector2& newBasisI, const Vector2& newBasisJ,
							   Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ );

Vector3 CrossProduct(const Vector3& from, const Vector3& to);


float	SmoothStart2( float t ); // 2nd-degree smooth start (a.k.a. “quadratic ease in”)
float	SmoothStart3( float t ); // 3rd-degree smooth start (a.k.a. “cubic ease in”)
float	SmoothStart4( float t ); // 4th-degree smooth start (a.k.a. “quartic ease in”)
float	SmoothStop2( float t ); // 2nd-degree smooth start (a.k.a. “quadratic ease out”)
float	SmoothStop3( float t ); // 3rd-degree smooth start (a.k.a. “cubic ease out”)
float	SmoothStop4( float t ); // 4th-degree smooth start (a.k.a. “quartic ease out”)
float	SmoothStep3( float t ); // 3rd-degree smooth start/stop (a.k.a. “smoothstep”)


//   7 --- 6
//  /|    /|
// 3 +-- 2 |
// | 4 --| 5
// |/    |/
// 0 --- 1
std::vector<Vector3> GetPointsForProjection(const Matrix4& projection);
Vector3				 GetFrustumCenter(const std::vector<Vector3>& frustumPoints);
float				 GetFrustumEncompassingRadius(const std::vector<Vector3>& frustumPoints);


bool Quadratic( float a, float b, float c, float& out_root1, float& out_root2 );

// DOWNWARD GRAVITY IS POSITIVE
Vector2 Trajectory_GetPositionAtTime( float gravity, float launchSpeed, float launchAngleDegrees, float time );
float	Trajectory_GetMinimumLaunchSpeed( float gravity, float distance );  
bool	Trajectory_GetLaunchAngles( float& out_angle1,
									float& out_angle2,
									float  gravity,           // gravity 
									float  launchSpeed,      // launch speed
									float  distance,          // horizontal displacement desired
									float  height = 0.0f );   // vertical displacement desired
float   Trajectory_GetMaxHeight( float gravity, float launchSpeed, float distance );
Vector2 Trajectory_GetLaunchVelocity( float gravity,
									  float apexHeight,      // must be greater than height
									  float distance,         // target distance
									  float height );         // target height
