#include "Engine/Rendering/MeshBuilder.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Rendering/VertexLayout.hpp"
#include "ThirdParty/MikkT/mikktspace.h"



void MeshBuilder::Initialize( unsigned int drawPrimitive, bool useIndices ) 
{
	m_drawInstruction.m_primitiveType = drawPrimitive; 
	m_drawInstruction.m_usingIndices= useIndices; 
	m_drawInstruction.m_startIndex = 0;
	m_drawInstruction.m_elementCount = 0;


	// What was the purpose of this? Shouldn't both of these always be zero at this point?
	/*if (use_indices) {
	m_draw.start_index = m_indices.size(); 
	} else {
	m_draw.start_index = m_vertices.size(); 
	}*/
}



void MeshBuilder::Finalize()
{
	unsigned int endIndex;
	if (m_drawInstruction.m_usingIndices)
	{
		endIndex = m_indices.size(); 
	} 
	else 
	{
		endIndex = m_vertices.size(); 
	}

	GenerateBounds();

	m_drawInstruction.m_elementCount = endIndex- m_drawInstruction.m_startIndex; 
}



void MeshBuilder::SetVertexColor( const RGBA& color ) 
{
	m_vertexStamp.m_color = color; 
}



void MeshBuilder::SetVertexUVs( const Vector2& uvs )
{
	m_vertexStamp.m_UVs = uvs; 
}



void MeshBuilder::SetVertexUVs( float u, float v )
{
	m_vertexStamp.m_UVs = Vector2(u,v); 
}



void MeshBuilder::SetVertexNormal( const Vector3& normal )
{
	m_vertexStamp.m_normal = normal;
}



void MeshBuilder::SetVertexNormal( float x, float y, float z )
{
	m_vertexStamp.m_normal = Vector3(x, y, z);
}



void MeshBuilder::SetVertexTangent( const Vector4& tangent )
{
	m_vertexStamp.m_tangent = tangent;
}



void MeshBuilder::SetVertexTangent( float x, float y, float z, float bitangentDirection)
{
	Vector4 tangent = Vector4(x,y,z,bitangentDirection);
	SetVertexTangent(tangent);
}



unsigned int MeshBuilder::PushVertex( const Vector3& position ) 
{
	m_vertexStamp.m_position = position; 
	m_vertices.push_back( m_vertexStamp ); 

	return m_vertices.size() - 1; 
}



unsigned int MeshBuilder::PushVertex( const Vector2& position )
{
	Vector3 position3D = Vector3(position.x, position.y, 0.0f);
	return PushVertex(position3D);
}



void MeshBuilder::PushIndex( unsigned int index )
{
	m_indices.push_back(index);
}



void MeshBuilder::AddTriangle( unsigned int index1, unsigned int index2, unsigned int index3 )
{
	PushIndex(index1);
	PushIndex(index2);
	PushIndex(index3);
}



// Adds 2 triangles as 123, 134
// Assumes CCW ordering
void MeshBuilder::AddQuad( unsigned int index1, unsigned int index2, unsigned int index3, unsigned int index4 )
{
	AddTriangle(index1, index2, index3);
	AddTriangle(index1, index3, index4);
}



void MeshBuilder::AddLine( unsigned int index1, unsigned int index2 )
{
	PushIndex(index1);
	PushIndex(index2);
}



void MeshBuilder::AddLineQuad( unsigned int index1, unsigned int index2, unsigned int index3, unsigned int index4 )
{
	AddLine(index1, index2);
	AddLine(index2, index3);
	AddLine(index3, index4);
	AddLine(index4, index1);
}



void MeshBuilder::AddLine( const Vector3& position1, const Vector3& position2 )
{
	PushVertex(position1);
	PushVertex(position2);
}



void MeshBuilder::AddBillboardedQuad(const Vector3& center, const Vector3& right, const Vector3& up, float size)
{
	float halfSize = size * 0.5f;
	Vector2 halfDim = Vector2(halfSize, halfSize);
	AddBillboardedQuad(center, right, up, halfDim);
}



void MeshBuilder::AddBillboardedQuad(const Vector3& center, const Vector3& right, const Vector3& up, const Vector2& halfDimensions, const Vector2& uvMins, const Vector2& uvMaxs, const RGBA& color)
{
	// 3 - 2
	// | / |
	// 0 - 1
	Vector3 positions[4];
	positions[0] = center + (right * -halfDimensions.x) + (up * -halfDimensions.y);
	positions[1] = center + (right * +halfDimensions.x) + (up * -halfDimensions.y);
	positions[2] = center + (right * +halfDimensions.x) + (up * +halfDimensions.y);
	positions[3] = center + (right * -halfDimensions.x) + (up * +halfDimensions.y);


	SetVertexColor(color);
	
	if (m_drawInstruction.IsIndexed())
	{

		SetVertexUVs(uvMins.x, uvMins.y);
		unsigned int index = PushVertex(positions[0]);

		SetVertexUVs(uvMaxs.x, uvMins.y);
		PushVertex(positions[1]);

		SetVertexUVs(uvMaxs.x, uvMaxs.y);
		PushVertex(positions[2]);

		SetVertexUVs(uvMins.x, uvMaxs.y);
		PushVertex(positions[3]);

		AddQuad(index + 0, index + 1, index + 2, index + 3);
	}
	else
	{
		// Tri 1 - 012
		SetVertexUVs(uvMins.x, uvMins.y);
		PushVertex(positions[0]);

		SetVertexUVs(uvMaxs.x, uvMins.y);
		PushVertex(positions[1]);

		SetVertexUVs(uvMaxs.x, uvMaxs.y);
		PushVertex(positions[2]);

		// Tri 2 - 023
		SetVertexUVs(uvMins.x, uvMins.y);
		PushVertex(positions[0]);

		SetVertexUVs(uvMaxs.x, uvMaxs.y);
		PushVertex(positions[2]);

		SetVertexUVs(uvMins.x, uvMaxs.y);
		PushVertex(positions[3]);
	}
}



Mesh* MeshBuilder::CreateMesh(const VertexLayout* layout)
{
	SubMesh* submesh = CreateSubMesh(layout); 
	Mesh* mesh = new Mesh();
	mesh->AddSubMesh(submesh);

	return mesh; 
}



SubMesh* MeshBuilder::CreateSubMesh(const VertexLayout* layout)
{
	SubMesh *mesh = new SubMesh(); 

	// Set Verticies
	size_t vertexBufferSizeInBytes = layout->GetStride() * m_vertices.size(); 
	void *buffer = malloc( vertexBufferSizeInBytes ); 
	layout->CopyVerticies( buffer, m_vertices.data(), m_vertices.size() ); 
	mesh->SetVertices(layout->GetStride(), m_vertices.size(), buffer);
	free( buffer ); 

	mesh->SetIndices( m_indices.size(), m_indices.data() ); 
	mesh->SetVertexLayout(layout);
	mesh->m_drawInstruction = m_drawInstruction;
	mesh->m_bounds = m_bounds;

	return mesh; 
}



void MeshBuilder::SetBounds(const AABB3& bounds)
{
	m_bounds = bounds;
}



void MeshBuilder::GenerateBounds()
{
	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		m_bounds.StretchToIncludePoint(m_vertices[i].m_position);
	}
}



int MeshBuilder::GetNumberOfTriangles() const
{
	return m_vertices.size() / 3;
}



VertexBuilder& MeshBuilder::GetVertex(unsigned int triangleNumber, unsigned int vertexNumber)
{
	TODO("Const version of GetVertex");
	size_t index = (triangleNumber * 3) + vertexNumber;
	return m_vertices[index];
}



std::vector<VertexBuilder>& MeshBuilder::GetVertices()
{
	return m_vertices;
}



std::vector<unsigned int>&  MeshBuilder::GetIndices()
{
	return m_indices;
}



//const VertexBuilder& MeshBuilder::GetVertex(unsigned int triangleNumber, unsigned int vertexNumber) const
//{
//	VertexBuilder& vb = GetVertex(triangleNumber, vertexNumber); 
//	return vb;
//}


// MikkT
int Mikkt_GetNumFaces(const SMikkTSpaceContext* pContext)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	return mb->GetNumberOfTriangles();
}

int Mikkt_GetNumVerticiesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
{
	UNUSED(pContext);
	UNUSED(iFace);
	// All of our faces are 3 vertices(triangles)
	return 3;
}

void Mikkt_GetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	Vector3 position = mb->GetVertex(iFace, iVert).m_position;
	fvPosOut[0] = position.x;
	fvPosOut[1] = position.y;
	fvPosOut[2] = position.z;
}

void Mikkt_GetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	Vector3 normal = mb->GetVertex(iFace, iVert).m_normal;
	fvNormOut[0] = normal.x;
	fvNormOut[1] = normal.y;
	fvNormOut[2] = normal.z;
}

void Mikkt_GetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	Vector2 uvs = mb->GetVertex(iFace, iVert).m_UVs;
	fvTexcOut[0] = uvs.x;
	fvTexcOut[1] = uvs.y;
}

void Mikkt_SetTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	VertexBuilder& vertex = mb->GetVertex(iFace, iVert);
	vertex.m_tangent = Vector4(fvTangent[0], fvTangent[1], fvTangent[2], fSign);
}

bool MeshBuilder::GenerateTangents()
{
	SMikkTSpaceInterface mikktInterface;
	mikktInterface.m_getNumFaces = Mikkt_GetNumFaces;

	mikktInterface.m_getNumVerticesOfFace = Mikkt_GetNumVerticiesOfFace;
	
	mikktInterface.m_getPosition = Mikkt_GetPosition;
	mikktInterface.m_getNormal = Mikkt_GetNormal;
	mikktInterface.m_getTexCoord = Mikkt_GetTexCoord;
	
	mikktInterface.m_setTSpaceBasic = Mikkt_SetTSpaceBasic;
	mikktInterface.m_setTSpace = nullptr; // Unused

	SMikkTSpaceContext mikktContext;
	mikktContext.m_pInterface = &mikktInterface;
	mikktContext.m_pUserData = (void*)this;

	bool succeeded = genTangSpaceDefault(&mikktContext); 
	return succeeded;
}