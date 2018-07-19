#pragma once

#include "Engine/Math/Vector4.hpp"

class RGBA
{
public:
	RGBA(); // WHITE
	explicit RGBA(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);
	void SetFromText(const char* text);

	void SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);
	void SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 1.0f);

	void	GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const;
	Vector4 GetAsVector4() const;

	void ScaleRGB(float rgbScale); // Scales (and clamps) RGB components, but not A
	void ScaleAlpha(float alphaScale); // Scales (and clamps) Alpha, RGB is untouched
	void ScaleColor(float rScale, float gScale, float bScale, float aScale);

	// FOR MAP KEYS ONLY
	bool operator<(const RGBA& compare) const;

	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

