#pragma once

#include <vector>

#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/AABB3.hpp"

class VertexLayout;



class MeshBuilder 
{
public:
	void	 Initialize( unsigned int drawPrimitive, bool useIndices );
	void	 Finalize();
	Mesh*	 CreateMesh(const VertexLayout* layout = Vertex_3DPCUTBN::GetLayout());
	SubMesh* CreateSubMesh(const VertexLayout* layout = Vertex_3DPCUTBN::GetLayout());

	void SetBounds(const AABB3& bounds);
	void GenerateBounds();

	void SetVertexColor( const RGBA& color );
	void SetVertexUVs( const Vector2& uvs );
	void SetVertexUVs( float u, float v );
	void SetVertexNormal( const Vector3& normal );
	void SetVertexNormal( float x, float y, float z );
	void SetVertexTangent( const Vector4& tangent );
	void SetVertexTangent( float x, float y, float z, float bitangentDirection);
	unsigned int PushVertex( const Vector3& position );
	unsigned int PushVertex( const Vector2& position );

	void PushIndex(	  unsigned int index );
	void AddTriangle( unsigned int index1, unsigned int index2, unsigned int index3 );
	void AddQuad(	  unsigned int index1, unsigned int index2, unsigned int index3, unsigned int index4 ); // Adds 2 triangles as 123, 134. Assumes CCW ordering
	void AddLine(	  unsigned int index1, unsigned int index2 );
	void AddLineQuad( unsigned int index1, unsigned int index2, unsigned int index3, unsigned int index4 );

	void AddLine( const Vector3& position1, const Vector3& position2 );
	void AddBillboardedQuad(const Vector3& center, const Vector3& right, const Vector3& up, float size);
	void AddBillboardedQuad(const Vector3& center, const Vector3& right, const Vector3& up, const Vector2& halfDimensions, const Vector2& uvMins = Vector2(0.0f, 0.0f), const Vector2& uvMaxs = Vector2(1.0f, 1.0f), const RGBA& color = RGBA());

	bool GenerateTangents();

	int GetNumberOfTriangles() const;
	VertexBuilder&		 GetVertex(unsigned int triangleNumber, unsigned int vertexNumber);
	//const VertexBuilder& GetVertex(unsigned int triangleNumber, unsigned int vertexNumber) const;
	std::vector<VertexBuilder>& GetVertices();
	std::vector<unsigned int>&  GetIndices();

private:
	VertexBuilder				m_vertexStamp;
	std::vector<VertexBuilder>	m_vertices; 
	std::vector<unsigned int>	m_indices; 
	AABB3						m_bounds;

	DrawInstruction				m_drawInstruction; 
};
