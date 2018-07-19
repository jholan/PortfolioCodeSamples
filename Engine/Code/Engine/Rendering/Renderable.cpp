#include "Engine/Rendering/Renderable.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/Material.hpp"



Renderable::~Renderable()
{
	if (GetMesh() != nullptr && !GetMesh()->IsTracked())
	{
		delete m_mesh;
	}

	for (int i = 0; i < (int)m_materials.size(); ++i)
	{
		delete m_materials[i];
	}
}

void Renderable::SetMesh(const std::string& filepath)
{
	m_mesh = Mesh::CreateOrGet(filepath);
}



void Renderable::SetMesh( const Mesh* mesh )
{
	m_mesh = mesh;
}



const Mesh* Renderable::GetMesh() const
{
	return m_mesh;
}



void Renderable::SetMaterial(const std::string& name, unsigned int index)
{
	Material* material = Material::Get(name);
	SetMaterial(material, index);
}



void Renderable::EnsureMaterialArrayCanHoldXMaterials(unsigned int count)
{
	unsigned int targetSize = count + 1;
	if (m_materials.size() >= targetSize)
	{
		// We are correctly sized
	}
	else
	{
		// resize and fill with nullptr
		unsigned int currSize = (unsigned int)m_materials.size();
		m_materials.reserve(targetSize);
		for (unsigned int i = currSize; i < targetSize; ++i)
		{
			m_materials.push_back(nullptr);
		}
	}
}



void Renderable::SetMaterial( Material* material, unsigned int index )
{
	EnsureMaterialArrayCanHoldXMaterials(index);

	if (m_materials[index] != nullptr)
	{
		delete m_materials[index];
	}
	m_materials[index] = material;

}



Material* Renderable::GetMaterial(unsigned int index) const
{
	Material* material = nullptr;

	size_t numMaterials = m_materials.size();
	if (index < numMaterials - 1)
	{
		// If it is a valid index get that material
		material = m_materials[index];
	}
	else if (numMaterials > 0)
	{
		// Otherwise return the last material in the array
		material = m_materials[numMaterials - 1];
	}

	return material;
}



void Renderable::SetModelMatrix( const Matrix4& model )
{
	m_localToWorldMatrix = model;
}



Matrix4 Renderable::GetModelMatrix() const
{
	return m_localToWorldMatrix;
}



bool Renderable::CastsShadow() const
{
	return m_shouldCastShadows;
}



void Renderable::SetShadowCastingState(bool shouldCastShadows)
{
	m_shouldCastShadows = shouldCastShadows;
}



bool Renderable::ReceivesShadows() const
{
	return m_shouldReveiveShadows;
}



void Renderable::SetShadowReceivingState(bool shouldReceiveShadows)
{
	m_shouldReveiveShadows = shouldReceiveShadows;
}
