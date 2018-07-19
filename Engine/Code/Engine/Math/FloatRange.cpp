#include "Engine/Math/FloatRange.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//------------------------------------------------------------------------
FloatRange::FloatRange(const FloatRange& copyFrom)
{
	min = copyFrom.min;
	max = copyFrom.max;
}


//------------------------------------------------------------------------
FloatRange::FloatRange(float initialMin, float initialMax)
{
	min = initialMin;
	max = initialMax;
}


//------------------------------------------------------------------------
FloatRange::FloatRange(float initialMinMax)
{
	min = initialMinMax;
	max = initialMinMax;
}


//------------------------------------------------------------------------
void FloatRange::SetFromText(const char* text)
{
	min = StringToFloat(text);
	max = min;

	const char* tilde = FindFirstCharacterInString(text, "~");
	if (tilde != nullptr)
	{
		max = StringToFloat(++tilde);
	}
}



bool FloatRange::IsInRange(float number, bool includeMin, bool includeMax) const
{

	bool inRange = false;

	if (number > min && number < max)
	{
		inRange = true;
	}

	if (includeMin && number == min)
	{
		inRange = true;
	}

	if (includeMax && number == max)
	{
		inRange = true;
	}

	return inRange;
}


//------------------------------------------------------------------------
float FloatRange::GetRandomInRange() const
{
	float randomNumber = GetRandomFloatInRange(min, max);
	return randomNumber;
}