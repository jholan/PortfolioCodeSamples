#include "Engine/Math/Sphere.hpp"



Sphere::Sphere()
	: m_center(Vector3::ZEROS)
	, m_radius(1.0f)
{

}



Sphere::Sphere(const Vector3& center, float radius)
	: m_center(center)
	, m_radius(radius)
{

}

Vector3 Sphere::GetCenter() const
{
	return m_center;
}



void Sphere::SetCenter(const Vector3& center)
{
	m_center = center;
}



float Sphere::GetRadius() const
{
	return m_radius;
}



void Sphere::SetRadius(float radius)
{
	m_radius = radius;
}
