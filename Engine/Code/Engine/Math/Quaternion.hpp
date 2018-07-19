#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/MathUtils.hpp"


class Quaternion
{
public:

	Vector3 imaginary;
	float   real;

	// Composition
	Quaternion();
	Quaternion( float initial_x, float initial_y, float initial_z, float initial_w );
	Quaternion( float initional_real, const Vector3& initial_imaginary );
	~Quaternion();

	// Operators
	Quaternion	operator-() const;
	bool		operator==( const Quaternion& rhs ) const;
	bool		operator!=( const Quaternion& rhs ) const;

	Vector4		AsVector4() const;

	void		Normalize();
	bool		IsUnit() const;

	float		GetMagnitude() const;
	float		GetMagnitudeSquared() const;

	Quaternion	GetConjugate() const;
	Quaternion	GetInverse() const;

	Vector3		GetEuler() const;
	Matrix4		GetMatrix4() const;

	void		Invert();

	Vector3		GetRight() const;
	Vector3		GetUp() const;
	Vector3		GetForward() const;



	static const Quaternion IDENTITY;

	static Quaternion FromMatrix( const Matrix4& mat ); 
	static Quaternion FromAxisAngle( const Vector3& axis, float angle_radians );
	static Quaternion FromEuler( const Vector3& euler );
	static Quaternion FromEuler( float x, float y, float z );

	static Quaternion LookAt( const Vector3& forward );
};


#include "Engine/Core/EngineCommon.h"
inline Quaternion operator*( const Quaternion& lhs, const Quaternion& rhs )
{
	// lhs = q
	// rhs = r

	Quaternion ret;
	ret.real = (lhs.real * rhs.real) - DotProduct(lhs.imaginary, rhs.imaginary);
	ret.imaginary = CrossProduct( lhs.imaginary, rhs.imaginary ) + (rhs.real * lhs.imaginary) + (lhs.real * rhs.imaginary);
	return ret;
}

//------------------------------------------------------------------------
inline Vector3 operator*(const Quaternion& q, const Vector3& v ) 
{
	Quaternion p = Quaternion( 0.0f, v );
	Quaternion result = q * p * q.GetInverse();

	return result.imaginary;
}

//------------------------------------------------------------------------
inline Vector4 operator*(const Quaternion& q, const Vector4& v) 
{
	Vector3 result = q * Vector3(v.x, v.y, v.z) ;
	return Vector4( result, v.w );
}



// Quaternion Utilities -----------------------------------------------------
Quaternion	QuaternionDifference( const Quaternion& a, const Quaternion& b ); 
float		QuaternionAngleInDegrees( const Quaternion& a, const Quaternion& b );
float		QuaternionDot( const Quaternion& a, const Quaternion& b );
Quaternion	QuaternionLog( const Quaternion& q );
Quaternion	QuaternionExp( const Quaternion& q );
Quaternion	QuaternionScale( const Quaternion& q, float s ); 
Quaternion	QuaternionPow( const Quaternion& q, float e );
Quaternion	Slerp( const Quaternion& a, const Quaternion& b, float t ); 
Quaternion	QuaternionRotateTorward( const Quaternion& start, const Quaternion& end, float maxAngleDegrees );
Quaternion	Interpolate( const Quaternion& a, const Quaternion& b, float t );
bool		QuaternionEquals( const Quaternion& a, const Quaternion& b );




void QuaternionTests();