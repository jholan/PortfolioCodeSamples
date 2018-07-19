#pragma once

class Vector2;



class IntVector2
{
public:
	~IntVector2() {};									// destructor: do nothing (for speed)
	IntVector2() {};									// default constructor: do nothing (for speed)
	IntVector2(const IntVector2& copyFrom);				// copy constructor (from another vec2)
	explicit IntVector2(int initialX, int initialY);	// explicit constructor (from x, y)
	explicit IntVector2(const Vector2& vec);
	void SetFromText(const char* text);

	// Operators
	const IntVector2 operator+(const IntVector2& vecToAdd) const;			// IntVector + IntVector
	const IntVector2 operator-(const IntVector2& vecToSubtract) const;	// IntVector - IntVector
	const IntVector2 operator*(int uniformScale) const;					// IntVector * int
	const IntVector2 operator/(int inverseScale) const;					// IntVector / int					// THIS FEELS WEIRD
	void operator+=(const IntVector2& vecToAdd);							// IntVector += IntVector
	void operator-=(const IntVector2& vecToSubtract);					// IntVector -= IntVector
	void operator*=(const int uniformScale);							// IntVector *= int
	void operator/=(const int uniformDivisor);							// IntVector /= int
	void operator=(const  IntVector2& copyFrom);							// IntVector = IntVector
	bool operator==(const IntVector2& compare) const;					// IntVector == IntVector
	bool operator!=(const IntVector2& compare) const;					// IntVector != IntVector
	friend const IntVector2 operator*(int uniformScale, const IntVector2& vecToScale);	// int * IntVector


	int x;
	int y;
};