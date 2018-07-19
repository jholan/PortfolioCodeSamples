#pragma once

#include "Engine/Math/Vector3.hpp"

class Ray3;
class Sphere;
class AABB3;


bool IntersectRayAndSphere(const Ray3& ray, const Sphere& sphere, Vector3& out_intersectionPoint, float& out_distance);
bool IntersectRayAndAABB3(const Ray3& ray, const AABB3& box, Vector3& out_intersectionPoint, float& out_distance);