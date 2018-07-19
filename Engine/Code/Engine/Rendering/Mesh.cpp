#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/Renderer.hpp"

#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/MeshBuilder.hpp"



DrawInstruction::DrawInstruction()
	: m_primitiveType(Renderer::PRIMITIVE_TRIANGLES)
	, m_startIndex(0)
	, m_elementCount(0) 
	, m_usingIndices(false)
{

}



void SubMesh::SetIndices( unsigned int count, unsigned int const* indices )
{
	m_indexBuffer.m_indexCount = count;
	m_indexBuffer.m_indexStride = sizeof(unsigned int);
	m_indexBuffer.CopyToGPU(m_indexBuffer.m_indexStride * m_indexBuffer.m_indexCount, indices);

	if (count > 0)
	{
		m_drawInstruction.m_usingIndices = true;
		m_drawInstruction.m_elementCount = count;
	}
}



void SubMesh::SetVertices( unsigned int count, const Vertex_3DPCU* vertices )
{
	m_vertexBuffer.m_vertexCount = count;
	m_vertexBuffer.m_vertexStride = sizeof(Vertex_3DPCU);
	m_vertexBuffer.CopyToGPU( m_vertexBuffer.m_vertexStride * m_vertexBuffer.m_vertexCount, vertices); 

	if (!m_drawInstruction.m_usingIndices)
	{
		m_drawInstruction.m_elementCount = count;
	}
}



void SubMesh::SetVertices( unsigned int vertexStride, unsigned int count, void const *data )
{
	m_vertexBuffer.m_vertexCount = count;
	m_vertexBuffer.m_vertexStride = vertexStride;
	m_vertexBuffer.CopyToGPU( m_vertexBuffer.m_vertexStride * m_vertexBuffer.m_vertexCount, data); 

	if (!m_drawInstruction.m_usingIndices)
	{
		m_drawInstruction.m_elementCount = count;
	}
}



void SubMesh::SetDrawInstruction( unsigned int drawPrimitive, bool useIndices, unsigned int startIndex, unsigned int elementCount)
{
	m_drawInstruction.m_primitiveType = drawPrimitive;
	m_drawInstruction.m_startIndex = startIndex;
	m_drawInstruction.m_elementCount = elementCount;
	m_drawInstruction.m_usingIndices = useIndices;
}



void SubMesh::SetVertexLayout(const VertexLayout* layout)
{
	m_vertexLayout = layout;
}



const VertexLayout* SubMesh::GetVertexLayout() const
{
	return m_vertexLayout;
}



const AABB3& SubMesh::GetBounds() const
{
	return m_bounds;
}



// ----------------------------------------------------------------------------------------------------------------------
// -- Mesh Cluster ------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------
Mesh::Mesh()
{

}



Mesh::~Mesh()
{
	for (int i = 0; i < (int)m_subMeshes.size(); ++i)
	{
		delete m_subMeshes[i];
	}
	m_subMeshes.clear();
}



void Mesh::SetIsTracked(bool isTracked)
{
	m_isTracked = isTracked;
}



bool Mesh::IsTracked() const
{
	return m_isTracked;
}



void Mesh::AddSubMesh(SubMesh* submesh)
{
	m_subMeshes.push_back(submesh);
}



void Mesh::AddSubMeshes(const std::vector<SubMesh*>& meshes)
{
	for (int i = 0; i < (int)meshes.size(); ++i)
	{
		m_subMeshes.push_back(meshes[i]);
	}
}



const SubMesh* Mesh::GetSubMesh(unsigned int index) const
{
	return m_subMeshes[index];
}



unsigned int Mesh::GetSubMeshCount() const
{
	return m_subMeshes.size();
}



const Mesh*	Mesh::CreateOrGet(const std::string& filepath)
{
	Mesh* mesh = nullptr;

	std::map<std::string, Mesh*>::iterator alreadyExists = s_loadedMeshes.find(filepath);
	if (alreadyExists == s_loadedMeshes.end())
	{
		// The texture doesn't exist.
		// Currently we treat all meshes that dont exist as obj files
		mesh = CreateClusterFromOBJ(filepath);
		if (mesh == nullptr)
		{
			mesh = CreateClusterFromOBJ(BAD_FILEPATH);
		}

		s_loadedMeshes[filepath] = mesh;
		mesh->SetIsTracked(true);
	}
	else
	{
		// The texture does exist.
		mesh = s_loadedMeshes[filepath];
	}

	return mesh;
}



const Mesh* Mesh::GetFSQ()
{
	if (s_fsq == nullptr)
	{
		MeshBuilder builder;
		builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, false);

		builder.SetVertexColor(RGBA());

		// 0 \
		// 1 - 2
		builder.SetVertexUVs(0.0f, 2.0f);
		builder.PushVertex(Vector2(-1.0f, 3.0f));

		builder.SetVertexUVs(0.0f, 0.0f);
		builder.PushVertex(Vector2(-1.0f, -1.0f));

		builder.SetVertexUVs(2.0f, 0.0f);
		builder.PushVertex(Vector2(3.0f, -1.0f));

		builder.Finalize();
		s_fsq = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	}

	return s_fsq;
}



std::map<std::string, Mesh*> Mesh::s_loadedMeshes;
Mesh*						 Mesh::s_fsq;
