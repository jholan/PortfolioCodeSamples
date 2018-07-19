#pragma once

#include "Engine/Math/Vector3.hpp"



class Ray3
{
public:
	Ray3(const Vector3& start, const Vector3& direction);

	Vector3 start; 
	Vector3 direction; 

	Vector3 GetPositionAtDistance( float t ) const;
};