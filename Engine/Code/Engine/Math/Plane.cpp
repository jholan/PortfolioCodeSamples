#include "Engine/Math/Plane.hpp"



Plane::Plane(const Vector3& norm, const Vector3& pos) 
{
	distance = DotProduct(norm, pos);
	normal = norm; 
}



Plane::Plane(const Vector3& a, const Vector3& b, const Vector3& c)
{
	Vector3 edge0 = b - a; 
	Vector3 edge1 = c - a; 

	normal = CrossProduct( edge1, edge0 );
	GUARANTEE_OR_DIE( !FloatEquals(normal.GetLengthSquared(), 0.0f), "" ); 
	normal.NormalizeAndGetLength();


	// Any point works as the will all project to the same distance along the normal
	distance = DotProduct( normal, a ); 
}



float Plane::GetDistance(const Vector3& pos) const
{
	float dist = DotProduct( pos, normal ); 
	return dist - distance; 
}



bool Plane::IsInFront(const Vector3& pos) const
{
	return GetDistance(pos) > 0.0f;
}



void Plane::FlipNormal()
{
	normal = -normal;
	distance = -distance; 
}