#pragma once

#include <map>
#include <string>
#include <vector>

#include "Engine/Rendering/RenderBuffer.h"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/AABB3.hpp"


class VertexBuffer : public RenderBuffer
{
public:
	unsigned int m_vertexCount;  
	unsigned int m_vertexStride; 
};

class IndexBuffer : public RenderBuffer
{
public:
	unsigned int m_indexStride; 
	unsigned int m_indexCount; 
};

struct DrawInstruction 
{
	DrawInstruction();

	bool IsIndexed() const { return m_usingIndices; };

	unsigned int m_primitiveType; 
	unsigned int m_startIndex; 
	unsigned int m_elementCount; 
	bool		 m_usingIndices; 
};

class SubMesh 
{
public:
	void SetIndices( unsigned int count, unsigned int const* indices );  
	void SetVertices( unsigned int count, const Vertex_3DPCU* vertices ); 
	void SetVertices( unsigned int vertexStride, unsigned int count, void const *data );  
	
	void SetDrawInstruction( unsigned int drawPrimitive, bool useIndices, unsigned int startIndex, unsigned int elementCount); 
	
	void SetVertexLayout(const VertexLayout* layout);
	const VertexLayout* GetVertexLayout() const;

	const AABB3& GetBounds() const;
	

public:
	VertexBuffer m_vertexBuffer; // vertices
	IndexBuffer m_indexBuffer;	 // indices

	// Draw Call 
	DrawInstruction m_drawInstruction; 
	const VertexLayout* m_vertexLayout;

	// Bounds
	AABB3 m_bounds;
	
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	void				SetIsTracked(bool isTracked); // DBAD
	bool				IsTracked() const;

	void				AddSubMesh(SubMesh* submesh);
	void				AddSubMeshes(const std::vector<SubMesh*>& subMeshes);

	const SubMesh*		GetSubMesh(unsigned int index) const;
	unsigned int		GetSubMeshCount() const;
	
	static const Mesh*	CreateOrGet(const std::string& filepath);
	static const Mesh*	GetFSQ(); 


private:
	std::vector<SubMesh*> m_subMeshes;
	bool				  m_isTracked = false;

	// Database
	static std::map<std::string, Mesh*> s_loadedMeshes;
	static Mesh*						s_fsq;

	static constexpr char* BAD_FILEPATH = "Data/BuiltIns/BAD_MESH.obj"; 
};