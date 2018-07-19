#include "Engine/Math/AABB3.hpp"

#include <limits>

#include "Engine/Math/MathUtils.hpp"



AABB3::AABB3()
{
	float inf = std::numeric_limits<float>::infinity();
	m_mins = Vector3(inf, inf, inf);
	m_maxs = Vector3(-inf, -inf, -inf);
}



AABB3::AABB3(const Vector3& mins, const Vector3& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}



Vector3 AABB3::GetCenter() const
{
	Vector3 dimensions = GetDimensions();
	Vector3 center = GetMins() + (dimensions * 0.5f);

	return center;
}



Vector3 AABB3::GetMins() const
{
	return m_mins;
}



Vector3 AABB3::GetMaxs() const
{
	return m_maxs;
}



Vector3 AABB3::GetDimensions() const
{
	Vector3 dimensions = m_maxs - m_mins;
	return dimensions;
}



Vector3 AABB3::GetHalfDimensions() const
{
	Vector3 halfDim = GetDimensions() * 0.5f;
	return halfDim;
}



void AABB3::StretchToIncludePoint(const Vector3& point)
{
	m_mins = Min( point, m_mins ); 
	m_maxs = Max( point, m_maxs ); 
}



AABB3 AABB3::FromMinsMaxs(const Vector3& mins, const Vector3& maxs)
{
	AABB3 box = AABB3(mins, maxs);
	return box;
}



AABB3 AABB3::FromCenterHalfDim(const Vector3& center, const Vector3& halfDim)
{
	Vector3 mins = center - halfDim;
	Vector3 maxs = center + halfDim;
	AABB3 box = FromMinsMaxs(mins, maxs);
	return box;
}
