#pragma once



class IntRange
{
public:
	~IntRange() {};										
	IntRange(){};										
	IntRange(const IntRange& copyFrom);					
	explicit IntRange(int initialMin, int initialMax);
	explicit IntRange(int initialMinMax);
	void SetFromText(const char* text);

	int GetRandomInRange() const;
	bool IsInRange(int number) const;

	int min;
	int max;
};