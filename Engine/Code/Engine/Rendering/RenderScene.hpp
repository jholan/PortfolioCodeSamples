#pragma once

#include <vector>

#include "Engine/Core/RGBA.hpp"

#include "Engine/Rendering/UniformBuffer.hpp"

class Renderable;
class Camera;
class Light;
class ParticleEmitter;
class StaticScene;



class RenderScene
{
public:
	// Camera
	void AddCamera(Camera* camera);  
	void RemoveCamera(Camera* camera);
	int  GetIndexOfCameraInScene(const Camera* camera) const;
	bool IsCameraInScene(const Camera* camera) const;
	const std::vector<Camera*>& GetCameras() const;
	std::vector<Camera*>&		GetMutableCameras();


	// Renderable
	void AddRenderable(Renderable* renderable); 
	void RemoveRenderable(Renderable* renderable);
	int  GetIndexOfRenderableInScene(const Renderable* renderable) const;
	bool IsRenderableInScene(const Renderable* renderable) const;
	const std::vector<Renderable*>& GetRenderables() const;


	// Light
	void AddLight(Light* light); 
	void RemoveLight(Light* light);
	int  GetIndexOfLightInScene(const Light* light) const;
	bool IsLightInScene(const Light* light) const;
	const std::vector<Light*>& GetLights() const;
	std::vector<Light*>& GetMutableLights(); 


	// The Sun
	void SetSun(const Vector3& direction, const RGBA& color, float intensity);
	void RemoveSun();
	bool HasSun() const;
	const Light* GetSun() const;
	Light* GetMutableSun();


	// Particle Emitter
	void AddParticleEmitter(ParticleEmitter* emitter);  
	void RemoveParticleEmitter(ParticleEmitter* emitter);
	int  GetIndexOfParticleEmitterInScene(const ParticleEmitter* emitter) const;
	bool IsParticleEmitterInScene(const ParticleEmitter* emitter) const;
	const std::vector<ParticleEmitter*>& GetParticleEmitters() const;


	// Static Scene
	void AddStaticScene(const StaticScene* scene);
	void RemoveStaticScene(const StaticScene* scene);


	// Fog
	void UpdateFog();

	void SetFogColor(const RGBA& color);
		 
	void SetFogDistances(float startDistance, float maxDistance);
	void SetFogStartDistance(float startDistance);
	void SetFogMaxDistance(float maxDistance);
		 
	void SetFogFactors(float nearFactor, float farFactor);
	void SetFogNearFactor(float nearFactor);
	void SetFogFarFactor(float farFactor);


private:
	std::vector<Renderable*>		m_renderables; 
	std::vector<Light*>				m_lights; 
	std::vector<Camera*>			m_cameras; 
	std::vector<ParticleEmitter*>	m_particleEmitters; 

	Light*							m_theSun;

	// Fog
	UniformBlock					m_fogBlock;
};