#pragma once

#include "Engine/Math/Vector3.hpp"



class RaycastHit3
{
public:
	RaycastHit3() : hit(false) {};
	~RaycastHit3() {};

	bool hit; 
	Vector3 position;
	Vector3 normal; 
};