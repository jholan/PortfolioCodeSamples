#pragma once

#include <vector>

#include "Engine/Math/Matrix4.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/Material.hpp"

#define MAX_LIGHTS (8)

class	RenderScene;
class	Camera;
class	Renderable;
class	Shader;
class	Mesh;
class	Light;
enum	RenderQueue;



class DrawCall 
{
public:
	Matrix4			m_model; 
	const SubMesh*	m_mesh; 
	Material*		m_material;
	// int multi pass pass_number

	unsigned int	m_lightCount; 
	const Light*	m_lights[MAX_LIGHTS]; 

	int				m_layer; 
	RenderQueue		m_queue;

	bool			m_receivesShadows = true;
};



class ForwardRenderPath
{
public:
	ForwardRenderPath();

	void Render(RenderScene* scene);
	void RenderForCamera(Camera* camera, RenderScene* scene);
	
	void ClearForCamera(Camera* camera);
	void SortDrawCalls(std::vector<DrawCall>& drawCalls, const Vector3& cameraForward);
	void ComputeMostContributingLights(DrawCall& drawCall, RenderScene* scene);

	void AddTemporaryRenderable(Renderable* renderable);
	void ClearAllTemporaryRenderables();


private:
	Mesh*	m_skyboxMesh;
	Shader* m_skyboxShader;

	std::vector<Renderable*> m_temporaryRendeables;
};