#pragma once

#include <vector>

#include "Engine/Core/RGBA.hpp"

class InputController;
class Game;


class App
{
public:
	// Lifetime
	App()  {}; 
	~App() {};

	bool Initialize();
	bool Destroy();

	// Core
	void RunInternalLoop();

	// Quit
	bool IsQuitting() const;
	void Quit();


private:
	// Internal Updating
	void RunFrame();
	void Update(float deltaSeconds);
	void Render() const;

	// Internal Quit
	bool m_isQuitting;

	// Debug ShaderPrograms
	const std::vector<const char*> shaderProgramsToModify = 
	{
		"Data/Shaders/Phong"
	};

	int debugShaderProgramIndex = 0;
	const std::vector<const char*> debugShaderPrograms = 
	{
		// Normal Shader
		"self",

		// Vertex Data
		"Data/Shaders/NormalView",
		"Data/Shaders/View_VertexTangent",
		"Data/Shaders/View_VertexBitangent",
		"Data/Shaders/View_UVs",

		// Textures
		"Data/Shaders/View_DiffuseTexture",
		"Data/Shaders/View_SurfaceNormal",

		// Lighting
		"Data/Shaders/View_DiffuseLight",
		"Data/Shaders/View_SpecularLight",
		"Data/Shaders/View_AllLight",

		// Surface
		"Data/Shaders/View_WorldNormal"
	};

	const std::vector<const char*> debugShaderProgramDisplayNames = 
	{
		// Normal Shader
		"Self",

		// Vertex Data
		"Vertex Normals",
		"Vertex Tangents",
		"Vertex Bitangents",
		"UVs",

		// Textures
		"Diffuse Texture",
		"Surface Normal",

		// Lighting
		"Diffuse Light",
		"Specular Light",
		"All Light",

		// Surface
		"World Normal"
	};
};