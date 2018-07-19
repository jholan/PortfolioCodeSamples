#pragma once



class IntVector3
{
public:
	~IntVector3() {};												// destructor: do nothing (for speed)
	IntVector3() {};												// default constructor: do nothing (for speed)
	IntVector3(const IntVector3& copyFrom);							// copy constructor (from another vec2)
	explicit IntVector3(int initialX, int initialY, int intitialZ);	// explicit constructor (from x, y)
	void SetFromText(const char* text);

	// Operators
	const IntVector3 operator+(const IntVector3& vecToAdd) const;						// IntVector + IntVector
	const IntVector3 operator-(const IntVector3& vecToSubtract) const;					// IntVector - IntVector
	const IntVector3 operator*(int uniformScale) const;									// IntVector * int
	const IntVector3 operator/(int inverseScale) const;									// IntVector / int					// THIS FEELS WEIRD
	void operator+=(const IntVector3& vecToAdd);										// IntVector += IntVector
	void operator-=(const IntVector3& vecToSubtract);									// IntVector -= IntVector
	void operator*=(const int uniformScale);											// IntVector *= int
	void operator/=(const int uniformDivisor);											// IntVector /= int
	void operator=(const  IntVector3& copyFrom);										// IntVector = IntVector
	bool operator==(const IntVector3& compare) const;									// IntVector == IntVector
	bool operator!=(const IntVector3& compare) const;									// IntVector != IntVector
	friend const IntVector3 operator*(int uniformScale, const IntVector3& vecToScale);	// int * IntVector


	int x;
	int y;
	int z;
};