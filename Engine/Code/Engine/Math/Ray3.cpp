#include "Engine/Math/Ray3.hpp"



Ray3::Ray3(const Vector3& s, const Vector3& d)
{
	start = s;
	direction = d.GetNormalized();
}



Vector3 Ray3::GetPositionAtDistance( float t ) const
{
	return start + direction * t; 
}