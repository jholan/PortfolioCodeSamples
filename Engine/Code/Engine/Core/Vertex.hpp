#pragma once

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

#include "Engine/Rendering/VertexAttribute.hpp"
#include "Engine/Rendering/VertexLayout.hpp"



struct VertexBuilder
{
public:
	VertexBuilder() : m_position(0.0f, 0.0f, 0.0f), m_color(), m_normal(0.0f, 0.0f, 0.0f), m_UVs(0.0f, 0.0f) {};

	Vector3 m_position; 
	RGBA	m_color; 
	Vector3 m_normal;
	Vector2 m_UVs;
	Vector4 m_tangent; // xyz tangent, w cotangent sign
};



struct Vertex_3DPCU
{
public:
	Vertex_3DPCU() {};
	Vertex_3DPCU(const Vector2& position2D, const RGBA& color, const Vector2& uvs = Vector2(0.0f, 0.0f));
	Vertex_3DPCU(const Vector3& position3D, const RGBA& color, const Vector2& uvs = Vector2(0.0f, 0.0f));
	~Vertex_3DPCU() {};

	Vector3 m_position;
	RGBA	m_color;
	Vector2 m_UVs;

	static const VertexLayout  s_layout; 
	static const VertexLayout* GetLayout() { return &s_layout; };


private:
	static const VertexAttribute		s_attributes[]; 
};



struct Vertex_3DPCUTBN
{
public:
	Vertex_3DPCUTBN() {};
	Vertex_3DPCUTBN(const Vector2& position2D, const RGBA& color, const Vector3& normal, const Vector2& uvs = Vector2(0.0f, 0.0f), const Vector4& tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	Vertex_3DPCUTBN(const Vector3& position3D, const RGBA& color, const Vector3& normal, const Vector2& uvs = Vector2(0.0f, 0.0f), const Vector4& tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	~Vertex_3DPCUTBN() {};

	Vector3 m_position;
	RGBA	m_color;
	Vector3 m_normal;
	Vector2 m_UVs;
	Vector4 m_tangent; // xyz tangent, w cotangent sign

	static const VertexLayout  s_layout; 
	static const VertexLayout* GetLayout() { return &s_layout; };


private:
	static const VertexAttribute		s_attributes[]; 
};