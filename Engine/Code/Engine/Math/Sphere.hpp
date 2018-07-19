#pragma once

#include "Engine/Math/Vector3.hpp"



class Sphere
{
public:
	Sphere();
	Sphere(const Vector3& center, float radius);

	Vector3 GetCenter() const;
	void	SetCenter(const Vector3& center);

	float	GetRadius() const;
	void	SetRadius(float radius);


private:
	Vector3 m_center;
	float	m_radius;
};