#pragma once
#include "Engine/Rendering/Camera.hpp"

#include "Engine/Math/FloatRange.hpp"


class OrbitCamera : public Camera
{
public:
	// whatever helpers you would prefer
	void SetTarget( Vector3 newTarget ); 
	void SetAzimuthAngleRange(const FloatRange& range);
	void SetSphericalCoordinates( float radius, float polarAngleDegrees, float azimuthAngleDegrees ); 
	void ApplyPolarRotation(float angleDegrees);
	void ApplyAzimuthRotation(float angleDegrees);

	float GetRadius() const;
	void SetRadius(float newRadius);

	float GetPolarAngleDegrees()   const;
	void SetPolarAngleDegrees(float newPolarAngle);

	float GetAzimuthAngleDegrees() const;
	void SetAzimuthAngleDegrees(float newAzimuthAngle);

private:
	void SetBasisMatricies();

	Vector3 m_target; 

	FloatRange m_azimuthAngleRange = FloatRange(-85.0f, 85.0f);

	float m_radius;					 // distance from target
	float m_polarAngleDegrees;		 // rotation around Y
	float m_azimuthAngleDegrees;     // rotation toward up after previous rotation


	// PUBLIC INHERITANCE
	//Matrix4 m_localToWorld;	
	//Matrix4 m_worldToLocal;	
	//Matrix4 m_projection;		
	//
	//FrameBuffer m_output;	
};
