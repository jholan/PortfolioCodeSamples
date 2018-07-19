#pragma once

class IntVector2;

//-----------------------------------------------------------------------------------------------
class Vector2
{
public:
	// Construction/Destruction
	~Vector2() {};											// destructor: do nothing (for speed)
	Vector2() {};											// default constructor: do nothing (for speed)
	Vector2(const Vector2& copyFrom);						// copy constructor (from another vec2)
	explicit Vector2(float initialX, float initialY);		// explicit constructor (from x, y)
	explicit Vector2(const IntVector2& ivec);
	void SetFromText(const char* text);

	// Operators
	const Vector2 operator+(const Vector2& vecToAdd) const;			// vec2 + vec2
	const Vector2 operator-(const Vector2& vecToSubtract) const;	// vec2 - vec2
	const Vector2 operator*(float uniformScale) const;				// vec2 * float
	const Vector2 operator/(float inverseScale) const;				// vec2 / float
	void operator+=(const Vector2& vecToAdd);						// vec2 += vec2
	void operator-=(const Vector2& vecToSubtract);					// vec2 -= vec2
	void operator*=(const float uniformScale);						// vec2 *= float
	void operator/=(const float uniformDivisor);					// vec2 /= float
	void operator=(const Vector2& copyFrom);						// vec2 = vec2
	bool operator==(const Vector2& compare) const;					// vec2 == vec2
	bool operator!=(const Vector2& compare) const;					// vec2 != vec2

	float GetLength() const;
	float GetLengthSquared() const; // faster than GetLength() since it skips the sqrtf()

	float NormalizeAndGetLength(); // normalize THIS vector and return the length before it was normalized.
	Vector2 GetNormalized() const; // return a new vector, which is a normalized copy of me
	Vector2 GetHadamard(const Vector2 & scaleFactors) const;

	float GetOrientationDegrees() const; // return 0 for east (5,0), 90 for north (0,8), etc.

	static Vector2 MakeDirectionAtDegrees(float degrees); // create vector at angle

	friend const Vector2 operator*(float uniformScale, const Vector2& vecToScale);	// float * vec2


public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x;
	float y;

	static const Vector2 ZEROS;
	static const Vector2 ONES;

	static const Vector2 X_AXIS;
	static const Vector2 Y_AXIS;

	static const Vector2 RIGHT;
	static const Vector2 FORWARD;
};


