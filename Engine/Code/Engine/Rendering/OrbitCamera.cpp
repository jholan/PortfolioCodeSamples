#include "Engine/Rendering/OrbitCamera.hpp"

#include "Engine/Math/MathUtils.hpp"



void OrbitCamera::SetTarget( Vector3 newTarget )
{
	m_target = newTarget;
	SetBasisMatricies();
}



void OrbitCamera::SetAzimuthAngleRange(const FloatRange& range)
{
	m_azimuthAngleRange = range;
}



void OrbitCamera::SetSphericalCoordinates( float radius, float polarAngleDegrees, float azimuthAngleDegrees )
{
	m_radius = radius;
	m_polarAngleDegrees = polarAngleDegrees;
	m_azimuthAngleDegrees = azimuthAngleDegrees;
	SetBasisMatricies();
}



void OrbitCamera::ApplyPolarRotation(float angleDegrees)
{
	m_polarAngleDegrees += angleDegrees;
	SetBasisMatricies();
}



void OrbitCamera::ApplyAzimuthRotation(float angleDegrees)
{
	m_azimuthAngleDegrees += angleDegrees;
	SetBasisMatricies();
}



float OrbitCamera::GetRadius() const
{
	return m_radius;
}



void OrbitCamera::SetRadius(float newRadius)
{
	m_radius = newRadius;
	SetBasisMatricies();
}



float OrbitCamera::GetPolarAngleDegrees() const
{
	return m_polarAngleDegrees;
}



void OrbitCamera::SetPolarAngleDegrees(float newPolarAngleDegrees)
{
	m_polarAngleDegrees = newPolarAngleDegrees;
	SetBasisMatricies();
}



float OrbitCamera::GetAzimuthAngleDegrees() const
{
	return m_azimuthAngleDegrees;
}


void OrbitCamera::SetAzimuthAngleDegrees(float newAzimuthAngleDegrees)
{
	m_azimuthAngleDegrees = newAzimuthAngleDegrees;
	SetBasisMatricies();
}



void OrbitCamera::SetBasisMatricies()
{
	while(m_polarAngleDegrees < 0.0f || m_polarAngleDegrees > 360.0f)
	{
		if (m_polarAngleDegrees < 0.0f)
		{
			m_polarAngleDegrees += 360.0f;
		}
		else if (m_polarAngleDegrees > 360.0f)
		{
			m_polarAngleDegrees -= 360.0f;
		}
	}

	m_azimuthAngleDegrees = ClampFloat(m_azimuthAngleDegrees, m_azimuthAngleRange.min, m_azimuthAngleRange.max);
	LookAt(m_target + ConvertSphericalToCartesian(m_radius, m_polarAngleDegrees, m_azimuthAngleDegrees), m_target);
}
