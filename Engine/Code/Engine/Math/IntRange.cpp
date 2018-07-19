#include "Engine/Math/IntRange.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//------------------------------------------------------------------------
IntRange::IntRange(const IntRange& copyFrom)
{
	min = copyFrom.min;
	max = copyFrom.max;
}


//------------------------------------------------------------------------
IntRange::IntRange(int initialMin, int initialMax)
{
	min = initialMin;
	max = initialMax;

	if (min > max)
	{
		int t = max;
		max = min;
		min = t;
	}
}


//------------------------------------------------------------------------
IntRange::IntRange(int initialMinMax)
{
	min = initialMinMax;
	max = initialMinMax;
}


//------------------------------------------------------------------------
void IntRange::SetFromText(const char* text)
{
	min = StringToInt(text);
	max = min;

	const char* tilde = FindFirstCharacterInString(text, "~");
	if (tilde != nullptr)
	{
		max = StringToInt(++tilde);
	}
}


//------------------------------------------------------------------------
int IntRange::GetRandomInRange() const
{
	int randomNumber = GetRandomIntInRange(min, max);
	return randomNumber;
}


//------------------------------------------------------------------------
bool IntRange::IsInRange(int number) const
{
	bool inRange = false;

	if (number >= min && number <= max)
	{
		inRange = true;
	}

	return inRange;
}