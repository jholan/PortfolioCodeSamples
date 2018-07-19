#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"


// COLUMN MAJOR
class Matrix4
{
public:
	Matrix4(); // default-construct to Identity matrix (via variable initialization)
	explicit Matrix4( const float* sixteenValuesBasisMajor ); // float[16] array in order Ix, Iy...
	explicit Matrix4( const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation=Vector2(0.f,0.f) );
	explicit Matrix4( const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation = Vector3::ZEROS);
	explicit Matrix4( const Vector4& iBasis, const Vector4& jBasis, const Vector4& kBasis, const Vector4& translation = Vector4::ZEROS);
	void SetFromText(const char* text);

	// Retrieve Basis Vectors
	Vector3 GetForward()	 const;
	Vector3 GetRight()		 const;
	Vector3 GetUp()			 const;
	Vector3 GetTranslation() const;

	Vector3 GetEulerAngles() const;

	// Accessors
	Vector2 TransformPosition2D( const Vector2& position2D ); // Written assuming z=0, w=1
	Vector2 TransformDisplacement2D( const Vector2& displacement2D ); // Written assuming z=0, w=0

	Vector3 TransformPosition( const Vector3& position );
	Vector4 operator*(const Vector4& vectorToMultiply) const;

	// Mutators
	void SetIdentity();
	void SetValues( const float* sixteenValuesBasisMajor ); // float[16] array in order Ix, Iy...
	void Append( const Matrix4& matrixToAppend ); // a.k.a. Concatenate (right-multiply)
	void Invert();
	Matrix4 operator*(const Matrix4 toAppend) const;
	Matrix4 GetTransposed() const;

	void RotateDegrees2D( float rotationDegreesAboutZ );
	void Translate2D( const Vector2& translation );
	void ScaleUniform2D( float scaleXY );
	void Scale2D( float scaleX, float scaleY );



	// Producers
	static Matrix4 MakeRotationDegrees2D( float rotationDegreesAboutZ );
	static Matrix4 MakeTranslation2D( const Vector2& translation );
	static Matrix4 MakeScaleUniform2D( float scaleXY );
	static Matrix4 MakeScale2D( float scaleX, float scaleY );
	static Matrix4 MakeOrtho2D( const Vector2& bottomLeft, const Vector2& topRight );

	static Matrix4 CreateRotationAroundXDegrees(float rotationDegrees);
	static Matrix4 CreateRotationAroundYDegrees(float rotationDegrees);
	static Matrix4 CreateRotationAroundZDegrees(float rotationDegrees);
	static Matrix4 CreateEulerRotationDegrees(const Vector3& eulerAnglesDegrees);
	static Matrix4 CreateTranslation(const Vector3& translation);
	static Matrix4 CreateUniformScale(float scaleXYZ);
	static Matrix4 CreateScale(float scaleX, float scaleY, float scaleZ);
	static Matrix4 CreateOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
	static Matrix4 CreatePerspectiveProjection( float fov_degrees, float aspect, float nz, float fz );
	static Matrix4 CreateLookAt(const Vector3& observerPosition, const Vector3& targetPosition, const Vector3& worldUp = Vector3(0.0f, 1.0f, 0.0f));

	static Matrix4 CreateLookAtInverse(const Vector3& observerPosition, const Vector3& targetPosition, const Vector3& worldUp = Vector3(0.0f, 1.0f, 0.0f));

	float Ix = 1.0f, Iy = 0.0f, Iz = 0.0f, Iw = 0.0f; // The first 4 floats are the first column
	float Jx = 0.0f, Jy = 1.0f, Jz = 0.0f, Jw = 0.0f;
	float Kx = 0.0f, Ky = 0.0f, Kz = 1.0f, Kw = 0.0f;
	float Tx = 0.0f, Ty = 0.0f, Tz = 0.0f, Tw = 1.0f;

private:
};