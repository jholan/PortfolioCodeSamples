#pragma once



class FloatRange
{
public:
	~FloatRange() {};										
	FloatRange() {};										
	FloatRange(const FloatRange& copyFrom);					
	explicit FloatRange(float initialMin, float initialMax);
	explicit FloatRange(float initialMinMax);
	void SetFromText(const char* text);

	bool IsInRange(float number, bool includeMin = true, bool includeMax = true) const;
	float GetRandomInRange() const;

	float min;
	float max;
};