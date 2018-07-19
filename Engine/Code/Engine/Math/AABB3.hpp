#pragma once

#include "Engine/Math/Vector3.hpp"



class AABB3 
{
public:
	AABB3();
	AABB3(const Vector3& mins, const Vector3& maxs);

	Vector3 GetCenter() const;
	Vector3 GetMins() const;
	Vector3 GetMaxs() const;
	Vector3 GetDimensions() const;
	Vector3 GetHalfDimensions() const;

	void	StretchToIncludePoint(const Vector3& point);


	static AABB3 FromMinsMaxs(const Vector3& mins, const Vector3& maxs);
	static AABB3 FromCenterHalfDim(const Vector3& center, const Vector3& halfDim);


private:
	Vector3 m_mins;
	Vector3 m_maxs;
};