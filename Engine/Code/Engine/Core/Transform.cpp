#include "Engine/Core/Transform.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.h"

#include "Engine/Math/Vector4.hpp"



Transform::Transform()
{
	m_position = DEFAULT_POSITION;
	m_scale = DEFAULT_SCALE;
	m_quatOrientation = DEFAULT_ORIENTATION;
}



Transform::Transform(const Vector3& position, const Vector3& eulerAngles, const Vector3& scale)
{
	SetLocalScale(scale);
	SetLocalEulerAngles(eulerAngles);
	SetLocalPosition(position);
}



Transform::Transform(const Matrix4& matrix)
{
	SetFromMatrix(matrix);
}



Transform::~Transform(){}




Transform* Transform::GetParent() const
{
	return m_parent;
}



void Transform::SetParent(Transform* parent)
{
	// Short Circuit
	if (parent == m_parent)
	{
		return;
	}


	// If our current parent is valid we need to unregister as its child
	if (m_parent != nullptr)
	{
		m_parent->RemoveChild(this);
	}

	if (parent != nullptr)
	{
		// If parent is a valid transform we need to register as its child
		// This sets our parent pointer
		parent->AddChild(this);
	}
	else
	{
		// Else set the pointer ourselves
		m_parent = nullptr;
	}
}



Transform* Transform::GetChild(unsigned int index)
{
	Transform* child = nullptr;

	if (index <m_children.size())
	{
		child = m_children[index];
	}

	return child;
}



const std::vector<Transform*>& Transform::GetChildren()
{
	return m_children;
}



void Transform::AddChild(Transform* child)
{
	// Short Circuit
	if (child == nullptr)
	{
		return;
	}


	m_children.push_back(child);
	if (child->GetParent() != nullptr)
	{
		child->GetParent()->RemoveChild(child);
	}
	child->m_parent = this;
}



void Transform::RemoveChild(Transform* child)
{
	for (int i = 0; i < (int)m_children.size(); ++i)
	{
		if (m_children[i] == child)
		{
			m_children.erase(m_children.begin() + i);
			child->m_parent = nullptr;
		}
	}
}



Vector3 Transform::GetRight() const
{
	Matrix4 localToWorld = GetLocalToWorldMatrix();
	Vector4 right = localToWorld * Vector4::X_AXIS;
	return Vector3(right.x, right.y, right.z);
}



Vector3 Transform::GetUp() const
{
	Matrix4 localToWorld = GetLocalToWorldMatrix();
	Vector4 up = localToWorld * Vector4::Y_AXIS;
	return Vector3(up.x, up.y, up.z);
}



Vector3 Transform::GetForward() const
{
	Matrix4 localToWorld = GetLocalToWorldMatrix();
	Vector4 forward = localToWorld * Vector4::Z_AXIS;
	return Vector3(forward.x, forward.y, forward.z);
}



Vector3 Transform::GetWorldPosition() const
{
	Vector4 worldPosition = GetLocalToWorldMatrix() * Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	return Vector3(worldPosition.x, worldPosition.y, worldPosition.z);
}



void Transform::SetWorldPosition(const Vector3& worldPosition)
{
	Vector4 parentSpacePosition = GetWorldToParentMatrix() * Vector4(worldPosition, 1.0f);
	m_position = Vector3(parentSpacePosition.x, parentSpacePosition.y, parentSpacePosition.z);
}



Vector3 Transform::GetLocalPosition() const
{
	return m_position;
}



void Transform::SetLocalPosition(const Vector3& newPos)
{
	m_position = newPos;
}



void Transform::TranslateLocal(const Vector3& translationVector)
{
	Vector4 translationVector4 = GetModelToParentMatrix() * Vector4(translationVector.x, translationVector.y, translationVector.z, 0);
	Vector3 translationVector3 = Vector3(translationVector4.x, translationVector4.y, translationVector4.z);

	m_position += translationVector3;
}



void Transform::TranslateWorld(const Vector3& translationVector)
{
	Vector4 translationVector4 = GetWorldToParentMatrix() * Vector4(translationVector.x, translationVector.y, translationVector.z, 0);
	Vector3 translationVector3 = Vector3(translationVector4.x, translationVector4.y, translationVector4.z);

	m_position += translationVector3;
}



Vector3 Transform::GetLocalScale() const
{
	return m_scale;
}



void Transform::SetLocalScale(const Vector3& newScale)
{
	m_scale = newScale;
}



void Transform::SetLocalScale(float xScale, float yScale, float zScale)
{
	Vector3 scale = Vector3(xScale, yScale, zScale);
	SetLocalScale(scale);
}



Quaternion Transform::GetLocalOrientation() const
{
	return m_quatOrientation;
}



void Transform::SetLocalOrientation(const Quaternion& newOrientation)
{
	m_quatOrientation = newOrientation;
}



void Transform::RotateAroundLocalAxis(const Vector3& localAxis, float theta)
{
	Vector4 parentAxis = GetModelToParentMatrix() * Vector4(localAxis, 0.0f);
	parentAxis.NormalizeAndGetLength();

	Quaternion parentRotation = Quaternion::FromAxisAngle(parentAxis.XYZ(), theta);
	m_quatOrientation = parentRotation * m_quatOrientation;
}



void Transform::RotateAroundWorldAxis(const Vector3& worldAxis, float theta)
{
	Vector4 parentAxis = GetWorldToParentMatrix() * Vector4(worldAxis, 0.0f);
	parentAxis.NormalizeAndGetLength();

	Quaternion parentRotation = Quaternion::FromAxisAngle(parentAxis.XYZ(), theta);
	m_quatOrientation = parentRotation * m_quatOrientation;
}



Vector3 Transform::GetLocalEulerAngles() const
{
	return m_quatOrientation.GetEuler();
	//return m_orientation;
}



void Transform::SetLocalEulerAngles(Vector3 euler)
{
	//m_orientation = euler;
	m_quatOrientation = Quaternion::FromEuler(euler);
}


#include "Engine/Rendering/DebugRender.hpp"
void Transform::RotateEuler(Vector3 deltaEuler)
{
	//m_orientation += euler;
	//Matrix4 worldToLocal = GetWorldToLocalMatrix();

	//Vector3 localZ = (GetWorldToLocalMatrix() * Vector4::Z_AXIS).XYZ();
	//localZ.NormalizeAndGetLength();
	//Quaternion rot_localZ = Quaternion::FromAxisAngle(localZ, euler.z);
	//m_quatOrientation = rot_localZ * m_quatOrientation;
	//
	//Vector3 localX = (GetWorldToLocalMatrix() * Vector4::X_AXIS).XYZ();
	//localX.NormalizeAndGetLength();
	//Quaternion rot_localX = Quaternion::FromAxisAngle(localX, euler.x);
	//m_quatOrientation = rot_localX * m_quatOrientation;
	//
	//Vector3 localY = (GetWorldToLocalMatrix() * Vector4::Y_AXIS).XYZ();
	//localY.NormalizeAndGetLength();
	//Quaternion rot_localY = Quaternion::FromAxisAngle(localY, euler.y);
	//m_quatOrientation = rot_localY * m_quatOrientation;

	//Vector3 eulers = GetLocalOrientation().GetMatrix4().GetEulerAngles();

	Vector3 eulers = m_quatOrientation.GetEuler();
	//Vector3 eulers = GetLocalToWorldMatrix().GetEulerAngles();
	DebugDraw_Log(0.0f, "RE       Eulers = (%.4f, %.4f, %.4f)", eulers.x, eulers.y, eulers.z);
	eulers += deltaEuler;
	DebugDraw_Log(0.0f, "RE Delta Eulers = (%.4f, %.4f, %.4f)", eulers.x, eulers.y, eulers.z);
	m_quatOrientation = Quaternion::FromEuler(eulers);

	//RotateAroundWorldAxis(Vector3::Z_AXIS, eulers.z);
	//RotateAroundWorldAxis(Vector3::X_AXIS, eulers.x);
	//RotateAroundWorldAxis(Vector3::Y_AXIS, eulers.y);
}


/*
* math::mat4 kaleidoscope::Transform::getModelToParentMatrix() const
*
* In: void :
* Out: mat4 : The transformation that takes a point in the objects model space and brings it into its parents space.
*/
Matrix4 Transform::GetModelToParentMatrix() const
{     
	//Matrix4 M = (Matrix4::CreateTranslation(m_position) * Matrix4::CreateEulerRotationDegrees(m_orientation) * Matrix4::CreateScale(m_scale.x, m_scale.y, m_scale.z));
	Matrix4 M = (Matrix4::CreateTranslation(m_position) * m_quatOrientation.GetMatrix4() * Matrix4::CreateScale(m_scale.x, m_scale.y, m_scale.z));
	return M;
}



Matrix4 Transform::GetWorldToParentMatrix() const
{
	Matrix4 M;
	if (GetParent() != nullptr)
	{
		M = GetParent()->GetWorldToLocalMatrix();
	} 
	else
	{
		M = Matrix4();
	}

	return M;
}



Matrix4 Transform::GetParentToWorldMatrix() const
{
	Matrix4 M;
	if (GetParent() != nullptr)
	{
		M = GetParent()->GetLocalToWorldMatrix();
	} 
	else
	{
		M = Matrix4();
	}

	return M;
}


/*
* math::mat4 kaleidoscope::Transform::getLocalToWorldMatrix() const
*
* In: void :
* Out: mat4 : The transformation that takes a point in the objects model space and brings it into world space.
*/
Matrix4 Transform::GetLocalToWorldMatrix() const
{
	Matrix4 M;
	M = GetParentToWorldMatrix() * GetModelToParentMatrix();

	return M;
}


/*
* math::mat4 kaleidoscope::Transform::getWorldToLocalMatrix() const
*
* In: void :
* Out: mat4 : The transformation that takes a point in world space and brings it into this transforms local space.
*/
Matrix4 Transform::GetWorldToLocalMatrix() const
{
	Matrix4 M = GetLocalToWorldMatrix();
	M.Invert();

	return M;
}



void Transform::SetFromMatrix(const Matrix4& matrix)
{
	m_scale.x = matrix.GetRight().GetLength();
	m_scale.y = matrix.GetUp().GetLength();
	m_scale.z = matrix.GetForward().GetLength();

	//m_orientation = matrix.GetEulerAngles();
	Vector4 i = Vector4(matrix.GetRight(), matrix.Iw).GetNormalized();
	Vector4 j = Vector4(matrix.GetUp(), matrix.Jw).GetNormalized();
	Vector4 k = Vector4(matrix.GetForward(), matrix.Kw).GetNormalized();
	Vector4 t = Vector4(matrix.GetTranslation(), matrix.Tw);
	Matrix4 M = Matrix4(i, j, k, t);
	m_quatOrientation = Quaternion::FromMatrix(M);
	
	m_position = matrix.GetTranslation();
}



/*
* void kaleidoscope::Transform::printState() const
* 
* Print the internal state of the transform
* NEEDS TO BE CHANGED
*
* Current idea is CSV string return
*/
void Transform::PrintState() const
{
	DebuggerPrintf("--Transform--\n   position = (%f, %f, %f)\n   scale = (%f, %f, %f)\n   orientation = i (%f, %f, %f), r = %f"
		, m_position.x, m_position.y, m_position.z
		, m_scale.x, m_scale.y, m_scale.z
		, m_quatOrientation.imaginary.x, m_quatOrientation.imaginary.y, m_quatOrientation.imaginary.z, m_quatOrientation.real);
}



const Vector3		Transform::DEFAULT_POSITION		= Vector3(0.0f, 0.0f, 0.0f);
const Vector3		Transform::DEFAULT_SCALE		= Vector3(1.0f, 1.0f, 1.0f);
const Quaternion	Transform::DEFAULT_ORIENTATION	= Quaternion();