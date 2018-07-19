#include "Engine/Core/Vertex.hpp"

#include "Engine/Rendering/GLFunctions.h"


//-----------------------------------------------------------------------------------------------
//---------------------------------Vertex_3DPCU--------------------------------------------------
//-----------------------------------------------------------------------------------------------
Vertex_3DPCU::Vertex_3DPCU(const Vector2& position2D, const RGBA& color, const Vector2& uvs)
	: m_position(position2D)
	, m_color(color)
	, m_UVs(uvs)
{
}



Vertex_3DPCU::Vertex_3DPCU(const Vector3& position3D, const RGBA& color, const Vector2& uvs)
	: m_position(position3D)
	, m_color(color)
	, m_UVs(uvs)
{
}



static void CopyVertex_3DPCUtoBuffer(  void* dst, const VertexBuilder* src, unsigned int count  ) 
{
	Vertex_3DPCU* buffer = (Vertex_3DPCU*)dst; 
	for (unsigned int i = 0; i < count; ++i)
	{
		const VertexBuilder& vertex = src[i];
		buffer[i] = Vertex_3DPCU(vertex.m_position, vertex.m_color, vertex.m_UVs);
	}
}



const VertexAttribute			Vertex_3DPCU::s_attributes[] = {
	VertexAttribute( "POSITION",    GL_FLOAT,           3, false, offsetof(Vertex_3DPCU, m_position) ),
	VertexAttribute( "COLOR",       GL_UNSIGNED_BYTE,   4, true,  offsetof(Vertex_3DPCU, m_color) ),
	VertexAttribute( "UV",          GL_FLOAT,           2, false, offsetof(Vertex_3DPCU, m_UVs) ),
	VertexAttribute::NULL_ATTRIBUTE
};
const VertexLayout				Vertex_3DPCU::s_layout = VertexLayout(sizeof(Vertex_3DPCU), s_attributes, CopyVertex_3DPCUtoBuffer);






//-----------------------------------------------------------------------------------------------
//---------------------------------Vertex_3DPCUN-------------------------------------------------
//-----------------------------------------------------------------------------------------------
Vertex_3DPCUTBN::Vertex_3DPCUTBN(const Vector2& position2D, const RGBA& color, const Vector3& normal, const Vector2& uvs, const Vector4& tangent)
	: m_position(position2D)
	, m_color(color)
	, m_normal(normal)
	, m_UVs(uvs)
	, m_tangent(tangent)
{
}



Vertex_3DPCUTBN::Vertex_3DPCUTBN(const Vector3& position3D, const RGBA& color, const Vector3& normal, const Vector2& uvs, const Vector4& tangent)
	: m_position(position3D)
	, m_color(color)
	, m_normal(normal)
	, m_UVs(uvs)
	, m_tangent(tangent)
{
}



static void CopyVertex_3DPCUTBNtoBuffer(  void* dst, const VertexBuilder* src, unsigned int count  ) 
{
	Vertex_3DPCUTBN* buffer = (Vertex_3DPCUTBN*)dst; 
	for (unsigned int i = 0; i < count; ++i)
	{
		const VertexBuilder& vertex = src[i];
		buffer[i] = Vertex_3DPCUTBN(vertex.m_position, vertex.m_color, vertex.m_normal, vertex.m_UVs, vertex.m_tangent);
	}
}



const VertexAttribute			Vertex_3DPCUTBN::s_attributes[] = {
	VertexAttribute( "POSITION",    GL_FLOAT,           3, false, offsetof(Vertex_3DPCUTBN, m_position) ),
	VertexAttribute( "COLOR",       GL_UNSIGNED_BYTE,   4, true,  offsetof(Vertex_3DPCUTBN, m_color) ),
	VertexAttribute( "NORMAL",      GL_FLOAT,			3, false, offsetof(Vertex_3DPCUTBN, m_normal) ),
	VertexAttribute( "UV",          GL_FLOAT,           2, false, offsetof(Vertex_3DPCUTBN, m_UVs) ),
	VertexAttribute( "TANGENT",     GL_FLOAT,           4, false, offsetof(Vertex_3DPCUTBN, m_tangent) ),
	VertexAttribute::NULL_ATTRIBUTE
};
const VertexLayout				Vertex_3DPCUTBN::s_layout = VertexLayout(sizeof(Vertex_3DPCUTBN), s_attributes, CopyVertex_3DPCUTBNtoBuffer);