#pragma once

#include <string>
#include <vector>

#include "Engine/Math/Matrix4.hpp"

class Mesh;
class Material;

class Renderable
{
public:
	Renderable() {};
	~Renderable();

	void		SetMesh(const std::string& filepath);
	void		SetMesh(const Mesh* mesh); 
	const Mesh*	GetMesh() const;

	void		SetMaterial(const std::string& name, unsigned int index = 0);
	void		SetMaterial(Material* material, unsigned int index = 0); 
	Material*	GetMaterial(unsigned int index = 0) const; 

	void		SetModelMatrix(const Matrix4& model); 
	Matrix4		GetModelMatrix() const; 

	bool		CastsShadow() const;
	void		SetShadowCastingState(bool shouldCastShadows);

	bool		ReceivesShadows() const;
	void		SetShadowReceivingState(bool shouldReceiveShadows);


private:
	void		EnsureMaterialArrayCanHoldXMaterials(unsigned int count);

	Matrix4					m_localToWorldMatrix = Matrix4(); 
	const Mesh*				m_mesh				 = nullptr; 
	std::vector<Material*>	m_materials;

	bool					m_shouldCastShadows = true;
	bool					m_shouldReveiveShadows = true;
};