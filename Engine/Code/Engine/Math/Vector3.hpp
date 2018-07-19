#pragma once

class Vector2;
class IntVector3;



//-----------------------------------------------------------------------------------------------
class Vector3
{
public:
	// Construction/Destruction
	~Vector3() {};															// destructor: do nothing (for speed)
	Vector3() {};															// default constructor: do nothing (for speed)
	Vector3(const Vector3& copyFrom);										// copy constructor (from another vec3)
	explicit Vector3(float initialX, float initialY, float initialZ);		// explicit constructor (from x, y, z)
	explicit Vector3(const Vector2& initialXY, float initialZ = 0.0f);
	explicit Vector3(const IntVector3& ivec);
	void SetFromText(const char* text);

	// Operators
	const Vector3 operator+(const Vector3& vecToAdd) const;			// Vector3 + Vector3
	const Vector3 operator-() const;								// -Vector3
	const Vector3 operator-(const Vector3& vecToSubtract) const;	// Vector3 - Vector3
	const Vector3 operator*(float uniformScale) const;				// Vector3 * float
	const Vector3 operator/(float inverseScale) const;				// Vector3 / float
	void operator+=(const Vector3& vecToAdd);						// Vector3 += Vector3
	void operator-=(const Vector3& vecToSubtract);					// Vector3 -= Vector3
	void operator*=(const float uniformScale);						// Vector3 *= float
	void operator/=(const float uniformDivisor);					// Vector3 /= float
	void operator=(const Vector3& copyFrom);						// Vector3 = Vector3
	bool operator==(const Vector3& compare) const;					// Vector3 == Vector3
	bool operator!=(const Vector3& compare) const;					// Vector3 != Vector3

	float GetLength() const;
	float GetLengthSquared() const; // faster than GetLength() since it skips the sqrtf()

	float NormalizeAndGetLength(); // normalize THIS vector and return the length before it was normalized.
	Vector3 GetNormalized() const; // return a new vector, which is a normalized copy of me
	Vector3 GetHadamard(const Vector3 & scaleFactors) const;

	// Swizzles
	Vector3 XZY() const;
	Vector2 XZ()  const;

	friend const Vector3 operator*(float uniformScale, const Vector3& vecToScale);	// float * Vector3


public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x;
	float y;
	float z;

	static const Vector3 ZEROS;
	static const Vector3 ONES;
	
	static const Vector3 X_AXIS;
	static const Vector3 Y_AXIS;
	static const Vector3 Z_AXIS;

	static const Vector3 RIGHT;
	static const Vector3 UP;
	static const Vector3 FORWARD;

	static const Vector3 I;
	static const Vector3 J;
	static const Vector3 K;
};