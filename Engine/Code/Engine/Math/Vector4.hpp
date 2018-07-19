#pragma once

class Vector3;



//-----------------------------------------------------------------------------------------------
class Vector4
{
public:
	// Construction/Destruction
	~Vector4() {};															// destructor: do nothing (for speed)
	Vector4() {};															// default constructor: do nothing (for speed)
	Vector4(const Vector4& copyFrom);										// copy constructor (from another vec3)
	explicit Vector4(float initialX, float initialY, float initialZ, float initialW);		// explicit constructor (from x, y, z)
	explicit Vector4(const Vector3& initialXYZ, float initialW = 1.0f);
	void SetFromText(const char* text);

	// Operators
	const Vector4 operator+(const Vector4& vecToAdd) const;			// Vector4 + Vector4
	const Vector4 operator-() const;								// -Vector4
	const Vector4 operator-(const Vector4& vecToSubtract) const;	// Vector4 - Vector4
	const Vector4 operator*(float uniformScale) const;				// Vector4 * float
	const Vector4 operator/(float inverseScale) const;				// Vector4 / float
	void operator+=(const Vector4& vecToAdd);						// Vector4 += Vector4
	void operator-=(const Vector4& vecToSubtract);					// Vector4 -= Vector4
	void operator*=(const float uniformScale);						// Vector4 *= float
	void operator/=(const float uniformDivisor);					// Vector4 /= float
	void operator=(const Vector4& copyFrom);						// Vector4 = Vector4
	bool operator==(const Vector4& compare) const;					// Vector4 == Vector4
	bool operator!=(const Vector4& compare) const;					// Vector4 != Vector4

	float GetLength() const;
	float GetLengthSquared() const; // faster than GetLength() since it skips the sqrtf()

	float NormalizeAndGetLength(); // normalize THIS vector and return the length before it was normalized.
	Vector4 GetNormalized() const; // return a new vector, which is a normalized copy of me
	Vector4 GetHadamard(const Vector4& scaleFactors) const;
	Vector3 XYZ() const;

	friend const Vector4 operator*(float uniformScale, const Vector4& vecToScale);	// float * Vector3


public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x;
	float y;
	float z;
	float w;

	static const Vector4 ZEROS;
	static const Vector4 ONES;

	static const Vector4 X_AXIS;
	static const Vector4 Y_AXIS;
	static const Vector4 Z_AXIS;

	static const Vector4 RIGHT;
	static const Vector4 UP;
	static const Vector4 FORWARD;

	static const Vector4 I;
	static const Vector4 J;
	static const Vector4 K;
};