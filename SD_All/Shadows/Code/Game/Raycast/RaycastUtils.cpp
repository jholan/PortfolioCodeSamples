#include "Game/Raycast/RaycastUtils.hpp"

#include <limits>

#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/Ray3.hpp"
#include "Engine/Math/Sphere.hpp"
#include "Engine/Math/AABB3.hpp"


bool IntersectRayAndSphere(const Ray3& ray, const Sphere& sphere, Vector3& out_intersectionPoint, float& out_distance)
{
	bool intersected = false;

	Vector3 toSphereCenter = sphere.GetCenter() - ray.start;
	
	float projectedLength = DotProduct(toSphereCenter, ray.direction);
	Vector3 projectedPosition = ray.GetPositionAtDistance(projectedLength);

	float distanceBetweenCenterAndProjectedPosition = (sphere.GetCenter() - projectedPosition).GetLength();
	if (distanceBetweenCenterAndProjectedPosition <= sphere.GetRadius())
	{
		intersected = true;

		//     *
		//    /|
		//   c b
		//  /  |
		// *-a-*
		// Pythagorean theorem a = a, b = distanceBetweenCenterAndProjectedPosition, c = sphere.GetRadius
		float a = SQRT( (sphere.GetRadius() * sphere.GetRadius()) - (distanceBetweenCenterAndProjectedPosition * distanceBetweenCenterAndProjectedPosition) );
		out_intersectionPoint = ray.GetPositionAtDistance(projectedLength - a);
		out_distance = projectedLength - a;
	}


	return intersected;
}



void Swap(float& lhs, float& rhs)
{
	float tmp = lhs;
	lhs = rhs;
	rhs = tmp;
}



bool IntersectRayAndAABB3(const Ray3& ray, const AABB3& box, Vector3& out_intersectionPoint, float& out_distance)
{
	float inf = std::numeric_limits<float>::infinity();
	Vector3 boxHalfDim = box.GetHalfDimensions();


	float tMin = -inf;
	float tMax = inf;

	Vector3 toBoxCenter = box.GetCenter() - ray.start;

	// X Side
	float e = DotProduct(Vector3::RIGHT, toBoxCenter);
	float f = DotProduct(Vector3::RIGHT, ray.direction);
	if (!FloatEquals(f, 0.0f))
	{
		float t1 = (e + boxHalfDim.x) / f;
		float t2 = (e - boxHalfDim.x) / f;
	
		if (t1 > t2)
		{
			Swap(t1, t2);
		}

		if (t1 > tMin)
		{
			tMin = t1;
		}

		if (t2 < tMax)
		{
			tMax = t2;
		}

		if (tMin > tMax)
		{
			return false;
		}

		if (tMax < 0.0f)
		{
			return false;
		}
	}
	else if ((-e - boxHalfDim.x > 0.0f) || (-e + boxHalfDim.x < 0.0f))
	{
		return false;
	}


	// Y Side
	e = DotProduct(Vector3::UP, toBoxCenter);
	f = DotProduct(Vector3::UP, ray.direction);
	if (!FloatEquals(f, 0.0f))
	{
		float t1 = (e + boxHalfDim.y) / f;
		float t2 = (e - boxHalfDim.y) / f;

		if (t1 > t2)
		{
			Swap(t1, t2);
		}

		if (t1 > tMin)
		{
			tMin = t1;
		}

		if (t2 < tMax)
		{
			tMax = t2;
		}

		if (tMin > tMax)
		{
			return false;
		}

		if (tMax < 0.0f)
		{
			return false;
		}
	}
	else if ((-e - boxHalfDim.y > 0.0f) || (-e + boxHalfDim.y < 0.0f))
	{
		return false;
	}



	// Y Side
	e = DotProduct(Vector3::FORWARD, toBoxCenter);
	f = DotProduct(Vector3::FORWARD, ray.direction);
	if (!FloatEquals(f, 0.0f))
	{
		float t1 = (e + boxHalfDim.z) / f;
		float t2 = (e - boxHalfDim.z) / f;

		if (t1 > t2)
		{
			Swap(t1, t2);
		}

		if (t1 > tMin)
		{
			tMin = t1;
		}

		if (t2 < tMax)
		{
			tMax = t2;
		}

		if (tMin > tMax)
		{
			return false;
		}

		if (tMax < 0.0f)
		{
			return false;
		}
	}
	else if ((-e - boxHalfDim.z > 0.0f) || (-e + boxHalfDim.z < 0.0f))
	{
		return false;
	}


	// Final tests
	if (tMin > 0.0f)
	{
		out_intersectionPoint = ray.GetPositionAtDistance(tMin);
		out_distance = tMin;
		return true;
	}
	else
	{
		out_intersectionPoint = ray.GetPositionAtDistance(tMax);
		out_distance = tMax;
		return true;
	}
}