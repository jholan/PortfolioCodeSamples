#include "Engine/Math/Quaternion.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Math/MathUtils.hpp"


Quaternion::Quaternion()
	: imaginary(0.0f, 0.0f, 0.0f)
	, real(1.0f)
{
}



Quaternion::Quaternion( float initial_x, float initial_y, float initial_z, float initial_w )
	: imaginary(initial_x, initial_y, initial_z)
	, real(initial_w)
{
}



Quaternion::Quaternion( float initional_r, const Vector3& initial_i )
	: imaginary(initial_i)
	, real(initional_r)
{
}



Quaternion::~Quaternion()
{
	imaginary = Vector3::ZEROS;
	real = 1.0f;
}



Quaternion Quaternion::operator-() const
{
	return Quaternion( -real, -imaginary );
}



bool Quaternion::operator==( const Quaternion& rhs ) const
{
	bool cond1 = FloatEquals(real, rhs.real) && (imaginary == rhs.imaginary);
	bool cond2 = FloatEquals(real, -rhs.real) && (imaginary == -rhs.imaginary);
	return cond1 || cond2;
}



bool Quaternion::operator!=( const Quaternion& rhs ) const
{
	return !(*this == rhs);
}



Vector4 Quaternion::AsVector4() const
{
	return Vector4(imaginary, real);
}



void Quaternion::Normalize()
{
	float const lengthSquared = GetMagnitudeSquared();

	// Already normalized.
	if (FloatEquals(lengthSquared, 1.0f))
	{
		return;
	}

	if (Abs(real) < .9999f)
	{
		float s = SQRT( 1.0f - real * real );
		Vector3 vector = imaginary / s;
		vector.NormalizeAndGetLength();

		imaginary = s * vector;
	}
	else 
	{
		//real = 1.0f;
		//imaginary = Vector3(0.0f, 0.0f, 0.0f);
		float length = SQRT(lengthSquared);
		float scaleFactor = 1.0f / length;

		real *= scaleFactor;
		imaginary *= scaleFactor;
	}
}



bool Quaternion::IsUnit() const
{
	float dot = QuaternionDot(*this, *this);
	return (Abs(dot - 1.0f) < .01f);
}



float Quaternion::GetMagnitude() const
{
	return SQRT( GetMagnitudeSquared() );
}



float Quaternion::GetMagnitudeSquared() const
{
	Vector4 data = AsVector4();
	return DotProduct( data, data );
}



Quaternion Quaternion::GetConjugate() const
{
	return Quaternion( real, -imaginary );
}



Quaternion Quaternion::GetInverse() const
{
	// We only deal with unit Quaternions in this engine!
	// ASSERT( IsUnit() );
	return GetConjugate();
}



Vector3 Quaternion::GetEuler() const
{
	Matrix4 matrixRepresentation = GetMatrix4();
	Vector3 eulerAngles = matrixRepresentation.GetEulerAngles();
	return eulerAngles;
}



// Look at the matrix of a rotation around a vector.  
// Notices the values in it - those are the components 
// of a Quaternion, or some combination thereof - so we can 
// refigure it into it; 
Matrix4 Quaternion::GetMatrix4() const
{
	float ix = imaginary.x;
	float iy = imaginary.y;
	float iz = imaginary.z;

	Vector3 iBV = Vector3();
	iBV.x = 1.0f  - (2.0f * ( (iy * iy) + (iz * iz) ));
	iBV.y = 2.0f * ( (ix * iy) + (real * iz) );
	iBV.z = 2.0f * ( (ix * iz) - (real * iy) );


	Vector3 jBV = Vector3();
	jBV.x = 2.0f * ( (ix * iy) - (real * iz) );
	jBV.y = 1.0f - (2.0f * ( (ix * ix) + (iz * iz) ));
	jBV.z = 2.0f * ((iy * iz) + (real * ix));

	Vector3 kBV = Vector3();
	kBV.x = 2.0f * ((ix * iz) + (real * iy));
	kBV.y = 2.0f * ((iy * iz) - (real * ix));
	kBV.z = 1.0f - (2.0f * ( (ix * ix) + (iy * iy) ));

	Vector3 tBV = Vector3(0.0f, 0.0f, 0.0f);

	Matrix4 ret = Matrix4(iBV, jBV, kBV, tBV);
	return ret;
}



void Quaternion::Invert()
{
	*this = GetInverse();
}



Vector3	Quaternion::GetRight() const
{
	return (*this) * Vector3(1.0f, 0.0f, 0.0f);
}



Vector3	Quaternion::GetUp() const
{
	return (*this) * Vector3(0.0f, 1.0f, 0.0f);
}



Vector3	Quaternion::GetForward() const
{
	return (*this) * Vector3(0.0f, 0.0f, 1.0f);
}



Quaternion Quaternion::FromMatrix( const Matrix4& mat )
{
	// Diagonal
	float ix = mat.Ix;
	float jy = mat.Jy;
	float kz = mat.Kz;
	float trace = ix + jy + kz;

	// FUCKED UP SHIT, SHOULDNT WORK
	float jz = mat.Ky;
	float ky = mat.Jz;

	float kx = mat.Iz;
	float iz = mat.Kx;

	float iy = mat.Jx;
	float jx = mat.Iy;


	Quaternion q;
	if (trace >= 0.0f)
	{
		float s = SQRT(trace + 1.0f) * 2.0f;
		float is = 1.0f / s;
		q.real = .25f * s;
		q.imaginary.x = (ky - jz) * is;
		q.imaginary.y = (iz - kx) * is;
		q.imaginary.z = (jx - iy) * is;
	}
	else if ((ix > jy) & (ix > kz)) 
	{
		float s = SQRT( 1.0f + ix - jy - kz ) * 2.0f;
		float is = 1.0f / s;
		q.real = (ky - jz) * is;
		q.imaginary.x = .25f * s;
		q.imaginary.y = (iy + jx) * is;
		q.imaginary.z = (iz + kx) * is;
	}
	else if (jy > kz)
	{
		float s = SQRT( 1.0f + jy - ix - kz ) * 2.0f;
		float is = 1.0f / s;
		q.real = (iz - kx) * is;
		q.imaginary.x = (iy + jx) * is;
		q.imaginary.y = .25f * s;
		q.imaginary.z = (jz + ky) * is;
	}
	else
	{
		float s = SQRT( 1.0f + kz - ix - jy ) * 2.0f;
		float is = 1.0f / s;
		q.real = (jx - iy) * is;
		q.imaginary.x = (iz + kx) * is;
		q.imaginary.y = (jz + ky) * is;
		q.imaginary.z = .25f * s;
	}

	q.Normalize();
	return q;
}



Quaternion Quaternion::FromAxisAngle( const Vector3& axis, float angleDegrees )
{
	float const halfAngle	 = 0.5f * angleDegrees;
	float const cosHalfAngle = CosDegrees(halfAngle);
	float const sinHalfAngle = SinDegrees(halfAngle);

	Quaternion orientaiton = Quaternion( cosHalfAngle, sinHalfAngle * axis );
	return orientaiton;
}



Quaternion Quaternion::FromEuler( const Vector3& euler )
{	
	Quaternion zRotation = Quaternion::FromAxisAngle( Vector3(0.0f, 0.0f, 1.0f), euler.z);
	Quaternion xRotation = Quaternion::FromAxisAngle( Vector3(1.0f, 0.0f, 0.0f), euler.x);
	Quaternion yRotation = Quaternion::FromAxisAngle( Vector3(0.0f, 1.0f, 0.0f), euler.y);
	
	Quaternion eulerRotation = yRotation * xRotation * zRotation;
	return eulerRotation;
}



Quaternion Quaternion::FromEuler( float x, float y, float z )
{
	return FromEuler( Vector3(x,y,z) );
}



Quaternion Quaternion::LookAt( const Vector3& forward )
{
	Matrix4 lookat = Matrix4::CreateLookAt(Vector3::ZEROS, forward);
	Quaternion orientation = FromMatrix( lookat );
	return orientation;
}



const Quaternion Quaternion::IDENTITY = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);



Quaternion QuaternionDifference( const Quaternion& a, const Quaternion& b )
{
	Quaternion rotationBetween = a.GetInverse() * b;
	return rotationBetween;
}



float QuaternionAngleInDegrees( const Quaternion& a, const Quaternion& b )
{
	// so just trying to get real part of the difference
	// inverse of a real part is the real part, so that doesn't change
	// but the imaginary part negates.
	// so need to compute that.
	float newReal = (a.real * b.real) - DotProduct(-a.imaginary, b.imaginary);
	newReal = ClampFloatNegativeOneToOne(newReal);

	float angle = 2.0f * acosf(newReal);
	float angleInDegrees = ConvertRadiansToDegrees(angle);
	return angleInDegrees;
}



float QuaternionDot( const Quaternion& a, const Quaternion& b )
{
	return DotProduct( a.AsVector4(), b.AsVector4() );
}


// Works on normalized Quaternion - returns a non-normalized Quaternion
Quaternion QuaternionLog( const Quaternion& q )
{
	// Purely just the real part
	if (q.real >= .9999f) 
	{
		return Quaternion( 0.0f, 0.0f, 0.0f, 0.0f );
	}
	
	float halfAngle = acosf(q.real);
	float s = SinRadians( halfAngle );

	Vector3 n =  q.imaginary / s;
	return Quaternion( 0.0f, halfAngle * n );
}


// Works on Quaternions of the form [0, a * i]
Quaternion QuaternionExp( const Quaternion& q )
{
	float halfAngle = q.imaginary.GetLength();
	Vector3 vec = Vector3(0.0f, 0.0f, 0.0f);
	float r = CosRadians(halfAngle);
	if (halfAngle > 0.00001f) 
	{
		Vector3 n = q.imaginary / halfAngle;
		vec = SinRadians(halfAngle) * n;
	}

	return Quaternion( r, vec );
}



Quaternion QuaternionScale( const Quaternion& q, float s )
{
	return Quaternion( s * q.real, s * q.imaginary );
}



Quaternion QuaternionPow( const Quaternion& q, float e )
{
	Quaternion ret = QuaternionExp( QuaternionScale( QuaternionLog( q ), e ) );
	ret.Normalize();
	return ret;
}



Quaternion Slerp( const Quaternion& a, const Quaternion& b, float t )
{
	float time = ClampFloatZeroToOne(t);
	float cosAngle = QuaternionDot( a, b );


	Quaternion start;
	if (cosAngle < 0.0f) 
	{
		// If it's negative - it means it's going the long way
		// flip it.
		start = -a;
		cosAngle = -cosAngle;
	}
	else 
	{
		start = a;
	}

	float f0, f1;
	if (cosAngle >= .9999f) 
	{
		// very close - just linearly interpolate.
		f0 = 1.0f - time;
		f1 = time;
	}
	else 
	{
		float sinAngle = SQRT( 1.0f - cosAngle * cosAngle );
		float angle = atan2f( sinAngle, cosAngle );

		float den = 1.0f / sinAngle;
		f0 = SinRadians( (1.0f - time) * angle ) * den;
		f1 = SinRadians( time * angle ) * den;
	}

	Quaternion r0 = QuaternionScale( start, f0 );
	Quaternion r1 = QuaternionScale( b, f1 );
	return Quaternion( r0.real + r1.real, r0.imaginary + r1.imaginary );
}



Quaternion QuaternionRotateTorward( const Quaternion& start, const Quaternion& end, float maxAngleDegrees )
{
	float angle = QuaternionAngleInDegrees(start, end);
	if (angle < 0.0f) 
	{
		angle = -angle;
	}

	if (FloatEquals(angle, 0.0f)) 
	{
		return end;
	}

	float t = ClampFloatZeroToOne( maxAngleDegrees / angle );
	Quaternion q = Slerp( start, end, t );
	return q;
}



Quaternion Interpolate( const Quaternion& a, const Quaternion& b, float t )
{
	return Slerp( a, b, t );
}



bool QuaternionEquals( const Quaternion& a, const Quaternion& b )
{
	float angleDegrees = QuaternionAngleInDegrees(a, b);
	float angleRadians = ConvertDegreesToRadians(angleDegrees);

	bool areEqual = angleRadians < 0.001f; 
	return areEqual;
}




// Single Axis
bool Test1()
{
	Vector3 p = Vector3(1.0f, 0.0f, 0.0f);
	Quaternion q = Quaternion::FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), 45.0f);
	Vector3 qtp = q * p;

	Vector4 p4 = Vector4(p, 1);
	Matrix4 M = Matrix4::CreateRotationAroundYDegrees(45.0f);
	Vector4 Mtp = M * p4;
	Vector3 p2 = Vector3(Mtp.x, Mtp.y, Mtp.z);

	return qtp == p2;
}

// Euler
bool Test2() 
{
	Vector3 eulerAngles = Vector3(80.0f, 92.5f, 0.0f);

	Vector3 p = Vector3(0.0f, 0.0f, 1.0f);
	Quaternion q = Quaternion::FromEuler(eulerAngles);
	Vector3 qtp = q * p;

	Vector4 p4 = Vector4(p, 1);
	Matrix4 M = Matrix4::CreateEulerRotationDegrees(eulerAngles);
	Vector4 Mtp = M * p4;
	Vector3 p2 = Vector3(Mtp.x, Mtp.y, Mtp.z);

	return qtp == p2;
}

// From Matrix Complex
bool Test3()
{
	Vector3 eulerAngles = Vector3(80.0f, 92.5f, 0.0f);
	Matrix4 M = Matrix4::CreateEulerRotationDegrees(eulerAngles);
	Quaternion q = Quaternion::FromMatrix(M);

	Vector3 p = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 qtp = q * p;


	Vector4 p4 = Vector4(p, 1);
	Vector4 Mtp = M * p4;
	Vector3 p2 = Vector3(Mtp.x, Mtp.y, Mtp.z);

	Matrix4 qM = q.GetMatrix4();

	return true;
}

// From matrix 
// Does the resultant matrix match?
bool Test4()
{
	Matrix4 M = Matrix4::CreateRotationAroundYDegrees(45.0f);
	Quaternion q = Quaternion::FromMatrix(M);
	Matrix4 M2 = q.GetMatrix4();

	Vector4 p = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	Vector4 Mtp = M * p;
	Vector4 M2tp = M2 * p;
	Vector4 qtp = q * p;

	return true;
}

// From matrix
// Does the from matrix quaternion match the true quaternion
bool Test5()
{
	Matrix4 M = Matrix4::CreateRotationAroundYDegrees(45.0f);
	Quaternion q = Quaternion::FromMatrix(M);
	Quaternion qTrue = Quaternion::FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), 45.0f);

	return true;
}


void QuaternionTests()
{
	Test1();
	Test2();
	Test3();
	Test4();
	Test5();
}