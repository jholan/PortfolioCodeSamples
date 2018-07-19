#pragma once

#include <vector>

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Quaternion.hpp"



class Transform
{
public:
	Transform();
	Transform(const Vector3& position, const Vector3& eulerAngles, const Vector3& scale);
	Transform(const Matrix4& matrix);
	~Transform();

	Transform*	GetParent() const;
	void		SetParent(Transform* parent = nullptr);

	Transform*						GetChild(unsigned int index);
	const std::vector<Transform*>&	GetChildren();
	void							AddChild(Transform* child);
	void							RemoveChild(Transform* child);

	Vector3		GetRight() const;
	Vector3		GetUp() const;
	Vector3		GetForward() const;

	Vector3		GetWorldPosition() const;
	void		SetWorldPosition(const Vector3& worldPosition);
	void		TranslateWorld(const Vector3& translationVector);

	Vector3		GetLocalPosition() const;
	void		SetLocalPosition(const Vector3& localPosition);
	void		TranslateLocal(const Vector3& translationVector);

	Vector3		GetLocalScale() const;
	void		SetLocalScale(const Vector3& newScale);
	void		SetLocalScale(float xScale, float yScale, float zScale);

	Quaternion	GetLocalOrientation() const;
	void		SetLocalOrientation(const Quaternion& newOrientation);
	void		RotateAroundLocalAxis(const Vector3& localAxis, float theta);
	void		RotateAroundWorldAxis(const Vector3& worldAxis, float theta);

	Vector3		GetLocalEulerAngles() const; 
	void		SetLocalEulerAngles(Vector3 euler); 
	void		RotateEuler(Vector3 euler); 

	Matrix4		GetModelToParentMatrix() const;
	Matrix4		GetWorldToParentMatrix() const;
	Matrix4		GetParentToWorldMatrix() const;

	Matrix4		GetLocalToWorldMatrix() const;
	Matrix4		GetWorldToLocalMatrix() const;

	void		SetFromMatrix(const Matrix4& matrix);

	void		PrintState() const;


private:
	Transform*				m_parent = nullptr;
	std::vector<Transform*>	m_children;

	Vector3					m_position;
	//Vector3					m_orientation;
	Quaternion				m_quatOrientation;
	Vector3					m_scale;


	static const Vector3	DEFAULT_POSITION;
	static const Quaternion	DEFAULT_ORIENTATION;
	static const Vector3	DEFAULT_SCALE;
};