#pragma once

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector3.hpp"


class Plane
{
public:
	Plane(const Vector3& norm, const Vector3& pos);
	Plane(const Vector3& a, const Vector3& b, const Vector3& c); // CCW

	float GetDistance(const Vector3& pos) const;
	bool IsInFront(const Vector3& pos) const;
	void FlipNormal();

	Vector3 normal;
	float distance; 
};