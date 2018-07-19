#include "Engine/Rendering/RenderScene.hpp"

#include "Engine/Rendering/Light.hpp"
#include "Engine/Rendering/ParticleEmitter.hpp"
#include "Engine/Core/StaticScene.hpp"

#include "Engine/Rendering/Renderer.hpp"

extern Renderer* g_theRenderer;

struct FogUniformBlock
{
	Vector4 	m_fogColor;

	float 		m_fogNearPlane;		// What depth to start the fog at
	float		m_fogFarPlane;		// What depth to end the fog at
	float		m_fogNearFactor;	// Amount of fog at the near plane
	float		m_fogFarFactor; 	// Amount of fog at the far plane
};



//-------------------------------------------------------------------------------------------------------
void RenderScene::AddCamera(Camera* camera)
{
	bool alreadyAdded = IsCameraInScene(camera);
	if (!alreadyAdded)
	{
		m_cameras.push_back(camera);
	}
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::RemoveCamera(Camera* camera)
{
	int index = GetIndexOfCameraInScene(camera);
	if (index > 0)
	{
		m_cameras.erase(m_cameras.begin() + index);
	}
}


//-------------------------------------------------------------------------------------------------------
int RenderScene::GetIndexOfCameraInScene(const Camera* camera) const
{
	int index = -1;

	for (int i = 0; i < (int)m_cameras.size(); ++i)
	{
		if (m_cameras[i] == camera)
		{
			index = i;
			break;
		}
	}

	return index;
}


//-------------------------------------------------------------------------------------------------------
bool RenderScene::IsCameraInScene(const Camera* camera) const
{
	bool inScene = false;

	int index = GetIndexOfCameraInScene(camera);
	if (index >= 0)
	{
		inScene = true;
	}

	return inScene;
}


//-------------------------------------------------------------------------------------------------------
const std::vector<Camera*>& RenderScene::GetCameras() const
{
	return m_cameras;
}


//-------------------------------------------------------------------------------------------------------
std::vector<Camera*>& RenderScene::GetMutableCameras()
{
	return m_cameras;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::AddRenderable(Renderable* renderable)
{
	bool alreadyAdded = IsRenderableInScene(renderable);
	if (!alreadyAdded)
	{
		m_renderables.push_back(renderable);
	}
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::RemoveRenderable(Renderable* renderable)
{
	int index = GetIndexOfRenderableInScene(renderable);
	if (index > 0)
	{
		m_renderables.erase(m_renderables.begin() + index);
	}
}


//-------------------------------------------------------------------------------------------------------
int RenderScene::GetIndexOfRenderableInScene(const Renderable* renderable) const
{
	int index = -1;

	for (int i = 0; i < (int)m_renderables.size(); ++i)
	{
		if (m_renderables[i] == renderable)
		{
			index = i;
			break;
		}
	}

	return index;
}


//-------------------------------------------------------------------------------------------------------
bool RenderScene::IsRenderableInScene(const Renderable* renderable) const
{
	bool inScene = false;

	int index = GetIndexOfRenderableInScene(renderable);
	if (index >= 0)
	{
		inScene = true;
	}

	return inScene;
}


//-------------------------------------------------------------------------------------------------------
const std::vector<Renderable*>& RenderScene::GetRenderables() const
{
	return m_renderables;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::AddLight(Light* light)
{
	bool alreadyAdded = IsLightInScene(light);
	if (!alreadyAdded)
	{
		m_lights.push_back(light);
	}
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::RemoveLight(Light* light)
{
	int index = GetIndexOfLightInScene(light);
	if (index >= 0)
	{
		m_lights.erase(m_lights.begin() + index);
	}
}


//-------------------------------------------------------------------------------------------------------
int RenderScene::GetIndexOfLightInScene(const Light* light) const
{
	int index = -1;

	for (int i = 0; i < (int)m_lights.size(); ++i)
	{
		if (m_lights[i] == light)
		{
			index = i;
			break;
		}
	}

	return index;
}


//-------------------------------------------------------------------------------------------------------
bool RenderScene::IsLightInScene(const Light* light) const
{
	bool inScene = false;

	int index = GetIndexOfLightInScene(light);
	if (index >= 0)
	{
		inScene = true;
	}

	return inScene;
}


//-------------------------------------------------------------------------------------------------------
const std::vector<Light*>& RenderScene::GetLights() const
{
	return m_lights;
}


//-------------------------------------------------------------------------------------------------------
std::vector<Light*>& RenderScene::GetMutableLights()
{
	return m_lights;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::SetSun(const Vector3& direction, const RGBA& color, float intensity)
{
	if (GetSun() != nullptr)
	{
		RemoveSun();
	}

	m_theSun = new Light();
	m_theSun->InitializeSunLight(direction, color, intensity);
}


//-------------------------------------------------------------------------------------------------------
const Light* RenderScene::GetSun() const
{
	return m_theSun;
}



//-------------------------------------------------------------------------------------------------------
Light* RenderScene::GetMutableSun()
{
	return m_theSun;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::RemoveSun()
{
	if (m_theSun != nullptr)
	{
		delete m_theSun;
		m_theSun = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------------
bool RenderScene::HasSun() const
{
	bool hasSun = false;

	if (m_theSun != nullptr)
	{
		hasSun = true;
	}

	return hasSun;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::AddParticleEmitter(ParticleEmitter* emitter)
{
	bool alreadyAdded = IsParticleEmitterInScene(emitter);
	if (!alreadyAdded)
	{
		m_particleEmitters.push_back(emitter);
		m_renderables.push_back(emitter->GetRenderable());
	}
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::RemoveParticleEmitter(ParticleEmitter* emitter)
{
	int index = GetIndexOfParticleEmitterInScene(emitter);
	if (index >= 0)
	{
		m_particleEmitters.erase(m_particleEmitters.begin() + index);
	}

	RemoveRenderable(emitter->GetRenderable());
}


//-------------------------------------------------------------------------------------------------------
int  RenderScene::GetIndexOfParticleEmitterInScene(const ParticleEmitter* emitter) const
{
	int index = -1;

	for (int i = 0; i < (int)m_particleEmitters.size(); ++i)
	{
		if (m_particleEmitters[i] == emitter)
		{
			index = i;
			break;
		}
	}

	return index;
}


//-------------------------------------------------------------------------------------------------------
bool RenderScene::IsParticleEmitterInScene(const ParticleEmitter* emitter) const
{
	bool inScene = false;

	int index = GetIndexOfParticleEmitterInScene(emitter);
	if (index >= 0)
	{
		inScene = true;
	}

	return inScene;
}


//-------------------------------------------------------------------------------------------------------
const std::vector<ParticleEmitter*>& RenderScene::GetParticleEmitters() const
{
	return m_particleEmitters;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::AddStaticScene(const StaticScene* scene)
{
	// Add Renderables
	const std::vector<Renderable*>& renderables = scene->GetRenderables();
	m_renderables.reserve(m_renderables.size() + renderables.size());
	for (int i = 0; i < (int)renderables.size(); ++i)
	{
		m_renderables.push_back(renderables[i]);
	}

	// Add Lights
	const std::vector<Light*>& lights = scene->GetLights();
	m_lights.reserve(m_lights.size() + lights.size());
	for (int i = 0; i < (int)lights.size(); ++i)
	{
		m_lights.push_back(lights[i]);
	}
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::RemoveStaticScene(const StaticScene* scene)
{
	// Remove Renderables
	const std::vector<Renderable*>& renderables = scene->GetRenderables();
	for (int i = 0; i < (int)renderables.size(); ++i)
	{
		RemoveRenderable(renderables[i]);
	}

	// Remove Lights
	const std::vector<Light*>& lights = scene->GetLights();
	for (int i = 0; i < (int)lights.size(); ++i)
	{
		RemoveLight(lights[i]);
	}
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::UpdateFog()
{
	g_theRenderer->BindUniformBlock(m_fogBlock, Renderer::FOG_BUFFER_BIND_POINT);
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::SetFogColor(const RGBA& color)
{
	FogUniformBlock* block = m_fogBlock.As<FogUniformBlock>();
	Vector4 fogColor = color.GetAsVector4();
	block->m_fogColor = fogColor;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::SetFogDistances(float startDistance, float maxDistance)
{
	FogUniformBlock* block = m_fogBlock.As<FogUniformBlock>();
	block->m_fogNearPlane = startDistance;
	block->m_fogFarPlane = maxDistance;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::SetFogStartDistance(float startDistance)
{
	FogUniformBlock* block = m_fogBlock.As<FogUniformBlock>();
	block->m_fogNearPlane = startDistance;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::SetFogMaxDistance(float maxDistance)
{
	FogUniformBlock* block = m_fogBlock.As<FogUniformBlock>();
	block->m_fogFarPlane = maxDistance;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::SetFogFactors(float nearFactor, float farFactor)
{
	FogUniformBlock* block = m_fogBlock.As<FogUniformBlock>();
	block->m_fogNearFactor = nearFactor;
	block->m_fogFarFactor = farFactor;
}



//-------------------------------------------------------------------------------------------------------
void RenderScene::SetFogNearFactor(float nearFactor)
{
	FogUniformBlock* block = m_fogBlock.As<FogUniformBlock>();
	block->m_fogNearFactor = nearFactor;
}


//-------------------------------------------------------------------------------------------------------
void RenderScene::SetFogFarFactor(float farFactor)
{
	FogUniformBlock* block = m_fogBlock.As<FogUniformBlock>();
	block->m_fogFarFactor = farFactor;
}
