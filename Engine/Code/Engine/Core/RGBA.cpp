#include "Engine/Core/RGBA.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------------------------
RGBA::RGBA() : r(255), g(255), b(255), a(255)
{
}


//-----------------------------------------------------------------------------------------------------------------
RGBA::RGBA(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte) 
	: r(redByte)
	, g(greenByte)
	, b(blueByte)
	, a(alphaByte)
{
}


//-----------------------------------------------------------------------------------------------------------------
void RGBA::SetFromText(const char* text)
{
	const char* start = FindFirstCharacterInString(text, "0123456789");

	// We must have atleast 2 commas
	const char* comma1 = FindFirstCharacterInString(text, ",");
	if (comma1 == nullptr) { return; }

	const char* comma2 = FindFirstCharacterInString(comma1 + 1, ",");
	if (comma2 == nullptr) { return; }

	// Set RGB Channels
	r = (unsigned char)ClampInt(StringToInt(start), 0, 255);
	g = (unsigned char)ClampInt(StringToInt(comma1 + 1), 0, 255);
	b = (unsigned char)ClampInt(StringToInt(comma2 + 1), 0, 255);

	// Check for optional alpha 
	const char* comma3 = FindFirstCharacterInString(comma2 + 1, ",");
	if (comma3 != nullptr)
	{
		a = (unsigned char)ClampInt(StringToInt(comma3 + 1), 0, 255);
	}
}


//-----------------------------------------------------------------------------------------------------------------
void RGBA::SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
{
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}


//-----------------------------------------------------------------------------------------------------------------
void RGBA::SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha)
{
	r = (unsigned char)(normalizedRed * 255);
	g = (unsigned char)(normalizedGreen * 255);
	b = (unsigned char)(normalizedBlue * 255);
	a = (unsigned char)(normalizedAlpha * 255);
}


//-----------------------------------------------------------------------------------------------------------------
void RGBA::GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const
{
	out_normalizedRed = (float)r / 255.0f;
	out_normalizedGreen = (float)g / 255.0f;
	out_normalizedBlue = (float)b / 255.0f;
	out_normalizedAlpha = (float)a / 255.0f;
}


//-----------------------------------------------------------------------------------------------------------------
Vector4 RGBA::GetAsVector4() const
{
	float out_r, out_g, out_b, out_a;
	GetAsFloats(out_r, out_g, out_b, out_a);
	return Vector4(out_r, out_g, out_b, out_a);
}


//-----------------------------------------------------------------------------------------------------------------
unsigned char ScaleAndClamp(unsigned char value, float scale)
{
	float result = value * scale;
	if(result > 255.0f)
	{
		result = 255.0f;
	}
	return (unsigned char)result;
}


//-----------------------------------------------------------------------------------------------------------------
void RGBA::ScaleRGB(float rgbScale)
{
	r = ScaleAndClamp(r, rgbScale);
	g = ScaleAndClamp(g, rgbScale);
	b = ScaleAndClamp(b, rgbScale);
}


//-----------------------------------------------------------------------------------------------------------------
void RGBA::ScaleAlpha(float alphaScale)
{
	a = ScaleAndClamp(a, alphaScale);
}


//-----------------------------------------------------------------------------------------------------------------
void RGBA::ScaleColor(float rScale, float gScale, float bScale, float aScale)
{
	r = ScaleAndClamp(r, rScale);
	g = ScaleAndClamp(g, gScale);
	b = ScaleAndClamp(b, bScale);
	a = ScaleAndClamp(a, aScale);
}


//-----------------------------------------------------------------------------------------------------------------
bool RGBA::operator<(const RGBA& compare) const
{
	bool isLessThan = false;

	if(r < compare.r)
	{
		isLessThan = true;
	}
	else if (r == compare.r)
	{
		if (g < compare.g)
		{
			isLessThan = true;
		}
		else if (g == compare.g)
		{
			if (b < compare.b)
			{
				isLessThan = true;
			}
			else if (b == compare.b)
			{
				if (a < compare.a)
				{
					isLessThan = true;
				}
			}
		}
	}

	return isLessThan;
}
