#include "Engine/Math/Matrix4.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Math/MathUtils.hpp"



Matrix4::Matrix4()
{
	Ix = 1.0f;
	Iy = 0.0f;
	Iz = 0.0f;
	Iw = 0.0f;

	Jx = 0.0f;
	Jy = 1.0f;
	Jz = 0.0f;
	Jw = 0.0f;

	Kx = 0.0f;
	Ky = 0.0f;
	Kz = 1.0f;
	Kw = 0.0f;

	Tx = 0.0f;
	Ty = 0.0f;
	Tz = 0.0f;
	Tw = 1.0f;
}



Matrix4::Matrix4( const float* sixteenValuesBasisMajor )
{
	SetValues(sixteenValuesBasisMajor);
}



Matrix4::Matrix4( const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation)
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = 0.0f;
	Iw = 0.0f;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = 0.0f;
	Jw = 0.0f;

	Kx = 0.0f;
	Ky = 0.0f;
	Kz = 1.0f;
	Kw = 0.0f;

	Tx = translation.x;
	Ty = translation.y;
	Tz = 0.0f;
	Tw = 1.0f;
}



Matrix4::Matrix4( const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation)
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = iBasis.z;
	Iw = 0.0f;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = jBasis.z;
	Jw = 0.0f;

	Kx = kBasis.x;
	Ky = kBasis.y;
	Kz = kBasis.z;
	Kw = 0.0f;

	Tx = translation.x;
	Ty = translation.y;
	Tz = translation.z;
	Tw = 1.0f;
}



Matrix4::Matrix4( const Vector4& iBasis, const Vector4& jBasis, const Vector4& kBasis, const Vector4& translation)
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = iBasis.z;
	Iw = iBasis.w;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = jBasis.z;
	Jw = jBasis.w;

	Kx = kBasis.x;
	Ky = kBasis.y;
	Kz = kBasis.z;
	Kw = kBasis.w;

	Tx = translation.x;
	Ty = translation.y;
	Tz = translation.z;
	Tw = translation.w;
}



void Matrix4::SetFromText(const char* text)
{
	UNIMPLEMENTED();
	UNUSED(text);
}


// I
Vector3 Matrix4::GetRight() const
{
	Vector3 right = Vector3(Ix, Iy, Iz);
	return  right;
}


// J
Vector3 Matrix4::GetUp() const
{
	Vector3 up = Vector3(Jx, Jy, Jz);
	return  up;
}


// K
Vector3 Matrix4::GetForward() const
{
	Vector3 forward = Vector3(Kx, Ky, Kz);
	return  forward;
}


// T
Vector3 Matrix4::GetTranslation() const
{
	Vector3 translation = Vector3(Tx, Ty, Tz);
	return translation;
}



Vector3 Matrix4::GetEulerAngles() const
{
	TODO("ASinDeg -> de rad the function");
	float xRad;
	float yRad;
	float zRad;

	float sx = -Ky;
	sx = ClampFloat(sx, -1.0f, 1.0f);
	xRad = asinf(sx);

	float cx = cosf(xRad);
	if (!FloatEquals(cx, 0.0f))
	{
		yRad = atan2f(Kx, Kz);
		zRad = atan2f(Iy, Jy);
	}
	else
	{
		// Gimbal?
		zRad = 0.0f;
		yRad = atan2f(-Iz, Ix);
	}

	Vector3 eulerDegrees = Vector3(ConvertRadiansToDegrees(xRad), ConvertRadiansToDegrees(yRad), ConvertRadiansToDegrees(zRad));
	return eulerDegrees;
}



Vector2 Matrix4::TransformPosition2D( const Vector2& position2D )
{
	Vector2 transformedPosition;
	float Pz = 0.0f;
	float Pw = 1.0f;

	transformedPosition.x = (Ix * position2D.x) + (Jx * position2D.y) + (Kx * Pz) + (Tx * Pw);
	transformedPosition.y = (Iy * position2D.x) + (Jy * position2D.y) + (Ky * Pz) + (Ty * Pw);

	return transformedPosition;
}



Vector2 Matrix4::TransformDisplacement2D( const Vector2& displacement2D )
{
	Vector2 transformedDisplacement;
	float Dz = 0.0f;
	float Dw = 0.0f;

	transformedDisplacement.x = (Ix * displacement2D.x) + (Jx * displacement2D.y) + (Kx * Dz) + (Tx * Dw);
	transformedDisplacement.y = (Iy * displacement2D.x) + (Jy * displacement2D.y) + (Ky * Dz) + (Ty * Dw);

	return transformedDisplacement;
}



Vector3 Matrix4::TransformPosition( const Vector3& position )
{
	Vector3 transformedPosition;
	float Pw = 1.0f;

	transformedPosition.x = (Ix * position.x) + (Jx * position.y) + (Kx * position.z) + (Tx * Pw);
	transformedPosition.y = (Iy * position.x) + (Jy * position.y) + (Ky * position.z) + (Ty * Pw);
	transformedPosition.z = (Iz * position.x) + (Jz * position.y) + (Kz * position.z) + (Tz * Pw);

	return transformedPosition;

}



Vector4 Matrix4::operator*(const Vector4& vec) const
{
	Vector4 transformedVec;

	transformedVec.x = (Ix * vec.x) + (Jx * vec.y) + (Kx * vec.z) + (Tx * vec.w);
	transformedVec.y = (Iy * vec.x) + (Jy * vec.y) + (Ky * vec.z) + (Ty * vec.w);
	transformedVec.z = (Iz * vec.x) + (Jz * vec.y) + (Kz * vec.z) + (Tz * vec.w);
	transformedVec.w = (Iw * vec.x) + (Jw * vec.y) + (Kw * vec.z) + (Tw * vec.w);

	return transformedVec;
}



void Matrix4::SetIdentity()
{
	Ix = 1.0f; 
	Iy = 0.0f; 
	Iz = 0.0f; 
	Iw = 0.0f;

	Jx = 0.0f; 
	Jy = 1.0f; 
	Jz = 0.0f; 
	Jw = 0.0f;

	Kx = 0.0f; 
	Ky = 0.0f; 
	Kz = 1.0f; 
	Kw = 0.0f;
	
	Tx = 0.0f; 
	Ty = 0.0f; 
	Tz = 0.0f; 
	Tw = 1.0f;
}



void Matrix4::SetValues( const float* sixteenValuesBasisMajor )
{
	Ix = sixteenValuesBasisMajor[0];
	Iy = sixteenValuesBasisMajor[1];
	Iz = sixteenValuesBasisMajor[2];
	Iw = sixteenValuesBasisMajor[3];

	Jx = sixteenValuesBasisMajor[4];
	Jy = sixteenValuesBasisMajor[5];
	Jz = sixteenValuesBasisMajor[6];
	Jw = sixteenValuesBasisMajor[7];

	Kx = sixteenValuesBasisMajor[8];
	Ky = sixteenValuesBasisMajor[9];
	Kz = sixteenValuesBasisMajor[10];
	Kw = sixteenValuesBasisMajor[11];

	Tx = sixteenValuesBasisMajor[12];
	Ty = sixteenValuesBasisMajor[13];
	Tz = sixteenValuesBasisMajor[14];
	Tw = sixteenValuesBasisMajor[15];
}



void Matrix4::Append( const Matrix4& matrixToAppend )
{
	const Matrix4& B = matrixToAppend;
	Matrix4 result;

	result.Ix = (Ix * B.Ix) + (Jx * B.Iy) + (Kx * B.Iz) + (Tx * B.Iw);
	result.Iy = (Iy * B.Ix) + (Jy * B.Iy) + (Ky * B.Iz) + (Ty * B.Iw);
	result.Iz = (Iz * B.Ix) + (Jz * B.Iy) + (Kz * B.Iz) + (Tz * B.Iw);
	result.Iw = (Iw * B.Ix) + (Jw * B.Iy) + (Kw * B.Iz) + (Tw * B.Iw);

	result.Jx = (Ix * B.Jx) + (Jx * B.Jy) + (Kx * B.Jz) + (Tx * B.Jw);
	result.Jy = (Iy * B.Jx) + (Jy * B.Jy) + (Ky * B.Jz) + (Ty * B.Jw);
	result.Jz = (Iz * B.Jx) + (Jz * B.Jy) + (Kz * B.Jz) + (Tz * B.Jw);
	result.Jw = (Iw * B.Jx) + (Jw * B.Jy) + (Kw * B.Jz) + (Tw * B.Jw);

	result.Kx = (Ix * B.Kx) + (Jx * B.Ky) + (Kx * B.Kz) + (Tx * B.Kw);
	result.Ky = (Iy * B.Kx) + (Jy * B.Ky) + (Ky * B.Kz) + (Ty * B.Kw);
	result.Kz = (Iz * B.Kx) + (Jz * B.Ky) + (Kz * B.Kz) + (Tz * B.Kw);
	result.Kw = (Iw * B.Kx) + (Jw * B.Ky) + (Kw * B.Kz) + (Tw * B.Kw);

	result.Tx = (Ix * B.Tx) + (Jx * B.Ty) + (Kx * B.Tz) + (Tx * B.Tw);
	result.Ty = (Iy * B.Tx) + (Jy * B.Ty) + (Ky * B.Tz) + (Ty * B.Tw);
	result.Tz = (Iz * B.Tx) + (Jz * B.Ty) + (Kz * B.Tz) + (Tz * B.Tw);
	result.Tw = (Iw * B.Tx) + (Jw * B.Ty) + (Kw * B.Tz) + (Tw * B.Tw);

	this->SetValues(&result.Ix);
}



void Matrix4::Invert()
{
	const float* m = &Ix; // Matrix to invert
	float invOut[16];


		double inv[16];
		double det;
		int i;

		inv[0] = m[5]  * m[10] * m[15] - 
			m[5]  * m[11] * m[14] - 
			m[9]  * m[6]  * m[15] + 
			m[9]  * m[7]  * m[14] +
			m[13] * m[6]  * m[11] - 
			m[13] * m[7]  * m[10];

		inv[4] = -m[4]  * m[10] * m[15] + 
			m[4]  * m[11] * m[14] + 
			m[8]  * m[6]  * m[15] - 
			m[8]  * m[7]  * m[14] - 
			m[12] * m[6]  * m[11] + 
			m[12] * m[7]  * m[10];

		inv[8] = m[4]  * m[9] * m[15] - 
			m[4]  * m[11] * m[13] - 
			m[8]  * m[5] * m[15] + 
			m[8]  * m[7] * m[13] + 
			m[12] * m[5] * m[11] - 
			m[12] * m[7] * m[9];

		inv[12] = -m[4]  * m[9] * m[14] + 
			m[4]  * m[10] * m[13] +
			m[8]  * m[5] * m[14] - 
			m[8]  * m[6] * m[13] - 
			m[12] * m[5] * m[10] + 
			m[12] * m[6] * m[9];

		inv[1] = -m[1]  * m[10] * m[15] + 
			m[1]  * m[11] * m[14] + 
			m[9]  * m[2] * m[15] - 
			m[9]  * m[3] * m[14] - 
			m[13] * m[2] * m[11] + 
			m[13] * m[3] * m[10];

		inv[5] = m[0]  * m[10] * m[15] - 
			m[0]  * m[11] * m[14] - 
			m[8]  * m[2] * m[15] + 
			m[8]  * m[3] * m[14] + 
			m[12] * m[2] * m[11] - 
			m[12] * m[3] * m[10];

		inv[9] = -m[0]  * m[9] * m[15] + 
			m[0]  * m[11] * m[13] + 
			m[8]  * m[1] * m[15] - 
			m[8]  * m[3] * m[13] - 
			m[12] * m[1] * m[11] + 
			m[12] * m[3] * m[9];

		inv[13] = m[0]  * m[9] * m[14] - 
			m[0]  * m[10] * m[13] - 
			m[8]  * m[1] * m[14] + 
			m[8]  * m[2] * m[13] + 
			m[12] * m[1] * m[10] - 
			m[12] * m[2] * m[9];

		inv[2] = m[1]  * m[6] * m[15] - 
			m[1]  * m[7] * m[14] - 
			m[5]  * m[2] * m[15] + 
			m[5]  * m[3] * m[14] + 
			m[13] * m[2] * m[7] - 
			m[13] * m[3] * m[6];

		inv[6] = -m[0]  * m[6] * m[15] + 
			m[0]  * m[7] * m[14] + 
			m[4]  * m[2] * m[15] - 
			m[4]  * m[3] * m[14] - 
			m[12] * m[2] * m[7] + 
			m[12] * m[3] * m[6];

		inv[10] = m[0]  * m[5] * m[15] - 
			m[0]  * m[7] * m[13] - 
			m[4]  * m[1] * m[15] + 
			m[4]  * m[3] * m[13] + 
			m[12] * m[1] * m[7] - 
			m[12] * m[3] * m[5];

		inv[14] = -m[0]  * m[5] * m[14] + 
			m[0]  * m[6] * m[13] + 
			m[4]  * m[1] * m[14] - 
			m[4]  * m[2] * m[13] - 
			m[12] * m[1] * m[6] + 
			m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] + 
			m[1] * m[7] * m[10] + 
			m[5] * m[2] * m[11] - 
			m[5] * m[3] * m[10] - 
			m[9] * m[2] * m[7] + 
			m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] - 
			m[0] * m[7] * m[10] - 
			m[4] * m[2] * m[11] + 
			m[4] * m[3] * m[10] + 
			m[8] * m[2] * m[7] - 
			m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] + 
			m[0] * m[7] * m[9] + 
			m[4] * m[1] * m[11] - 
			m[4] * m[3] * m[9] - 
			m[8] * m[1] * m[7] + 
			m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] - 
			m[0] * m[6] * m[9] - 
			m[4] * m[1] * m[10] + 
			m[4] * m[2] * m[9] + 
			m[8] * m[1] * m[6] - 
			m[8] * m[2] * m[5];

		det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (det == 0)
			return;

		det = 1.0 / det;

		for (i = 0; i < 16; i++)
			invOut[i] = (float)(inv[i] * det);

		SetValues(invOut);
}



void Matrix4::RotateDegrees2D( float rotationDegreesAboutZ )
{
	Matrix4 rotationMatrix = MakeRotationDegrees2D(rotationDegreesAboutZ);
	Append(rotationMatrix);
}



void Matrix4::Translate2D( const Vector2& translation )
{
	Matrix4 translationMatrix = MakeTranslation2D(translation);
	Append(translationMatrix);
}



void Matrix4::ScaleUniform2D( float scaleXY )
{
	Matrix4 scaleMatrix = MakeScaleUniform2D(scaleXY);
	Append(scaleMatrix);
}



void Matrix4::Scale2D( float scaleX, float scaleY )
{
	Matrix4 scaleMatrix = MakeScale2D(scaleX, scaleY);
	Append(scaleMatrix);
}



Matrix4 Matrix4::operator*(const Matrix4 toAppend) const
{
	const Matrix4& B = toAppend;
	Matrix4 result;

	result.Ix = (Ix * B.Ix) + (Jx * B.Iy) + (Kx * B.Iz) + (Tx * B.Iw);
	result.Iy = (Iy * B.Ix) + (Jy * B.Iy) + (Ky * B.Iz) + (Ty * B.Iw);
	result.Iz = (Iz * B.Ix) + (Jz * B.Iy) + (Kz * B.Iz) + (Tz * B.Iw);
	result.Iw = (Iw * B.Ix) + (Jw * B.Iy) + (Kw * B.Iz) + (Tw * B.Iw);

	result.Jx = (Ix * B.Jx) + (Jx * B.Jy) + (Kx * B.Jz) + (Tx * B.Jw);
	result.Jy = (Iy * B.Jx) + (Jy * B.Jy) + (Ky * B.Jz) + (Ty * B.Jw);
	result.Jz = (Iz * B.Jx) + (Jz * B.Jy) + (Kz * B.Jz) + (Tz * B.Jw);
	result.Jw = (Iw * B.Jx) + (Jw * B.Jy) + (Kw * B.Jz) + (Tw * B.Jw);

	result.Kx = (Ix * B.Kx) + (Jx * B.Ky) + (Kx * B.Kz) + (Tx * B.Kw);
	result.Ky = (Iy * B.Kx) + (Jy * B.Ky) + (Ky * B.Kz) + (Ty * B.Kw);
	result.Kz = (Iz * B.Kx) + (Jz * B.Ky) + (Kz * B.Kz) + (Tz * B.Kw);
	result.Kw = (Iw * B.Kx) + (Jw * B.Ky) + (Kw * B.Kz) + (Tw * B.Kw);

	result.Tx = (Ix * B.Tx) + (Jx * B.Ty) + (Kx * B.Tz) + (Tx * B.Tw);
	result.Ty = (Iy * B.Tx) + (Jy * B.Ty) + (Ky * B.Tz) + (Ty * B.Tw);
	result.Tz = (Iz * B.Tx) + (Jz * B.Ty) + (Kz * B.Tz) + (Tz * B.Tw);
	result.Tw = (Iw * B.Tx) + (Jw * B.Ty) + (Kw * B.Tz) + (Tw * B.Tw);

	return result;
}



Matrix4 Matrix4::GetTransposed() const
{
	Matrix4 transposed = Matrix4();

	transposed.Ix = Ix;
	transposed.Jx = Iy;
	transposed.Kx = Iz;
	transposed.Tx = Iw;

	transposed.Iy = Jx;
	transposed.Jy = Jy;
	transposed.Ky = Jz;
	transposed.Ty = Jw;

	transposed.Iz = Kx;
	transposed.Jz = Ky;
	transposed.Kz = Kz;
	transposed.Tz = Kw;

	transposed.Iw = Tx;
	transposed.Jw = Ty;
	transposed.Kw = Tz;
	transposed.Tw = Tw;

	return transposed;
}



Matrix4 Matrix4::MakeRotationDegrees2D( float rotationDegreesAboutZ )
{
	Matrix4 rotationtMatrix = Matrix4();

	float sinOfRotation = SinDegrees(rotationDegreesAboutZ);
	float cosOfRotation = CosDegrees(rotationDegreesAboutZ);

	rotationtMatrix.Ix = cosOfRotation;
	rotationtMatrix.Iy = sinOfRotation;

	rotationtMatrix.Jx = -sinOfRotation;
	rotationtMatrix.Jy = cosOfRotation;

	return rotationtMatrix;
}



Matrix4 Matrix4::MakeTranslation2D( const Vector2& translation )
{
	Matrix4 translationMatrix = Matrix4();

	translationMatrix.Tx = translation.x;
	translationMatrix.Ty = translation.y;

	return translationMatrix;
}



Matrix4 Matrix4::MakeScaleUniform2D( float scaleXY )
{
	Matrix4 scaleMatrix = Matrix4();

	scaleMatrix.Ix = scaleXY;
	scaleMatrix.Jy = scaleXY;

	return scaleMatrix;
}



Matrix4 Matrix4::MakeScale2D( float scaleX, float scaleY )
{
	Matrix4 scaleMatrix = Matrix4();

	scaleMatrix.Ix = scaleX;
	scaleMatrix.Jy = scaleY;

	return scaleMatrix;
}



Matrix4 Matrix4::MakeOrtho2D( const Vector2& bottomLeft, const Vector2& topRight )
{
	AABB2D bounds = AABB2D(bottomLeft, topRight);
	Vector2 orthoDim = bounds.GetDimensions();
	Vector2 orthoHalfDim = orthoDim / 2.0f;

	Matrix4 orthoMatrix = MakeScale2D(1.0f / orthoHalfDim.x , 1.0f / orthoHalfDim.y);
	orthoMatrix.Kz = -2.0f; // Scale from 0-1 to -1-1 and invert pos/neg on z

	orthoMatrix.Translate2D(-1.0f * bounds.GetCenter());
	orthoMatrix.Tz = -1.0f;

	return orthoMatrix;
}



Matrix4 Matrix4::CreateRotationAroundXDegrees(float rotationDegrees)
{
	float cosDeg = CosDegrees(rotationDegrees);
	float sinDeg = SinDegrees(rotationDegrees);

	Matrix4 rotationAroundX = Matrix4(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, cosDeg, sinDeg), Vector3(0.0f, -sinDeg, cosDeg));
	return rotationAroundX;
}



Matrix4 Matrix4::CreateRotationAroundYDegrees(float rotationDegrees)
{
	float cosDeg = CosDegrees(rotationDegrees);
	float sinDeg = SinDegrees(rotationDegrees);

	Matrix4 rotationAroundY = Matrix4(Vector3(cosDeg, 0.0f, -sinDeg), Vector3(0.0f, 1.0f, 0.0f), Vector3(sinDeg, 0.0f, cosDeg));
	return rotationAroundY;
}



Matrix4 Matrix4::CreateRotationAroundZDegrees(float rotationDegrees)
{
	float cosDeg = CosDegrees(rotationDegrees);
	float sinDeg = SinDegrees(rotationDegrees);

	Matrix4 rotationAroundY = Matrix4(Vector3(cosDeg, sinDeg, 0.0f), Vector3(-sinDeg, cosDeg, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
	return rotationAroundY;
}



Matrix4 Matrix4::CreateEulerRotationDegrees(const Vector3& eulerAnglesDegrees)
{
	Matrix4 zRotation = CreateRotationAroundZDegrees(eulerAnglesDegrees.z);
	Matrix4 xRotation = CreateRotationAroundXDegrees(eulerAnglesDegrees.x);
	Matrix4 yRotation = CreateRotationAroundYDegrees(eulerAnglesDegrees.y);

	Matrix4 eulerTransform = yRotation * xRotation * zRotation;
	return eulerTransform;
}



Matrix4 Matrix4::CreateTranslation(const Vector3& translation)
{
	Matrix4 translationMatrix = Matrix4();

	translationMatrix.Tx = translation.x;
	translationMatrix.Ty = translation.y;
	translationMatrix.Tz = translation.z;

	return translationMatrix;
}



Matrix4 Matrix4::CreateUniformScale(float scaleXYZ)
{
	Matrix4 uniformScaleMatrix = Matrix4();

	uniformScaleMatrix.Ix = scaleXYZ;
	uniformScaleMatrix.Jy = scaleXYZ;
	uniformScaleMatrix.Kz = scaleXYZ;

	return uniformScaleMatrix;
}



Matrix4 Matrix4::CreateScale(float scaleX, float scaleY, float scaleZ)
{
	Matrix4 scaleMatrix = Matrix4();

	scaleMatrix.Ix = scaleX;
	scaleMatrix.Jy = scaleY;
	scaleMatrix.Kz = scaleZ;

	return scaleMatrix;
}



Matrix4 Matrix4::CreateOrthographicProjection(float left, float right, float bottom, float top, float near, float far)
{
	float xScale = 2.0f / (right - left);
	float yScale = 2.0f / (top - bottom);
	float zScale = 2.0f / (far - near);

	float xTranslation = -(left + right) / 2.0f;
	float yTranslation = -(bottom + top) / 2.0f;
	float zTranslation = -(near + far) / 2.0f;
	Vector3	translation = Vector3(xTranslation, yTranslation, zTranslation);

	Matrix4 orthographicProjection = CreateScale(xScale, yScale, zScale) * CreateTranslation(translation);
	return orthographicProjection;
}



Matrix4 Matrix4::CreatePerspectiveProjection( float fov_degrees, float aspect, float nz, float fz )
{
	float d = 1.0f / TanDegrees(fov_degrees);
	float q = 1.0f / (fz - nz); 

	Vector4 i = Vector4( d / aspect, 0,    0,                   0 );  
	Vector4 j = Vector4( 0,          d,    0,                   0 ); 
	Vector4 k = Vector4( 0,          0,    (nz + fz) * q,       1 );
	Vector4 t = Vector4( 0,          0,    -2.0f * nz * fz * q, 0 ); 

	return Matrix4( i, j, k, t ); 
}



Matrix4 Matrix4::CreateLookAt(const Vector3& observerPosition, const Vector3& targetPosition, const Vector3& worldUp)
{
	Vector3 directionToTarget = targetPosition - observerPosition;
	Vector3 forward = directionToTarget.GetNormalized();
	Vector3 right = CrossProduct(worldUp, forward);
	right.NormalizeAndGetLength();
	Vector3 up = CrossProduct(forward, right);

	Matrix4 lookAtMatrix = Matrix4(right, up, forward, observerPosition);
	return lookAtMatrix;
}



Matrix4 Matrix4::CreateLookAtInverse(const Vector3& observerPosition, const Vector3& targetPosition, const Vector3& worldUp)
{
	Vector3 directionToTarget = targetPosition - observerPosition;
	Vector3 forward = directionToTarget.GetNormalized();
	Vector3 right = CrossProduct(worldUp, forward);
	right.NormalizeAndGetLength();
	Vector3 up = CrossProduct(forward, right);

	// Rigid body inverse
	//	Transpose the rotation
	//	Negate the translation
	Vector3 transposedRight		= Vector3(right.x, up.x, forward.x);
	Vector3 transposedUp		= Vector3(right.y, up.y, forward.y);
	Vector3 transposedForward	= Vector3(right.z, up.z, forward.z);

	Matrix4 inverseRotation = Matrix4(transposedRight, transposedUp, transposedForward);
	Matrix4 inverseTranslation = CreateTranslation(-observerPosition);
	Matrix4 inverseLookAt = inverseRotation * inverseTranslation;
	return  inverseLookAt;
}
