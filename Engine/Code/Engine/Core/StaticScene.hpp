#pragma once

#include <string>
#include <vector>

#include "Engine/Math/Matrix4.hpp"

class Camera;
class Light;
class Renderable;


class StaticScene
{
public:
	StaticScene();
	~StaticScene();

	void							ImportFile(const std::string& filename);
	void							ProcessNodeForMeshData(const Matrix4& parentMatrix, void* node, void* scene);

	const std::vector<Camera*>&		GetCameras() const;
	const std::vector<Camera*>&		GetCameras();

	const std::vector<Light*>&		GetLights() const;
	const std::vector<Light*>&		GetLights();

	const std::vector<Renderable*>& GetRenderables() const;
	const std::vector<Renderable*>& GetRenderables();



private:
	std::vector<Camera*>			m_cameras;
	std::vector<Light*>				m_lights;
	std::vector<Renderable*>		m_renderables;
};