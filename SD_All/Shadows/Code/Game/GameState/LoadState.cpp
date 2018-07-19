#include "Game/GameState/LoadState.hpp"

#include <vector>

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Engine/Core/StringUtils.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Blackboard.hpp"
extern Blackboard g_gameConfigBlackboard;

#include "Engine/Rendering/Camera.hpp"

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Sampler.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
extern Window*		g_theWidow;
extern Renderer*	g_theRenderer;
extern AudioSystem* g_theAudio;

#include "Game/Game.hpp"



void LoadState::Initialize(Game* owner)
{
	GUARANTEE_OR_DIE(owner != nullptr, "LoadState must be initialized with a valid owner");

	// Basic State
	SetOwner(owner);
	SetName("LoadState");


	// Background
	const Texture* loadScreen = Texture::CreateOrGet(g_gameConfigBlackboard.GetValue("loadStateBackground", "Data/BuiltIns/DEFAULT_TEXTURE.png"));
	m_loadScreenMaterial = Material::FromShader("Default_2D");
	m_loadScreenMaterial->SetDiffuseTexture(loadScreen, TEXTURE_SAMPLER_TYPE_POINT);

	m_backgroundQuad = CreateAABB2D(AABB2D(Vector2(0.0f, 0.0f), (Vector2)g_theWindow->GetDimensions()));


	// Camera
	m_camera = new Camera();
	m_camera->SetClearColor(RGBA(0,0,0));
	m_camera->SetProjection(Matrix4::MakeOrtho2D(Vector2(0.0f, 0.0f), (Vector2)g_theWindow->GetDimensions()));
	m_camera->SetColorTarget(g_theRenderer->GetDefaultColorTarget());
	m_camera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
}



void LoadState::Destroy()
{
	// Camera
	delete m_camera;
	m_camera = nullptr;

	// Background
	delete m_loadScreenMaterial;
	m_loadScreenMaterial = nullptr;

	delete m_backgroundQuad;
	m_backgroundQuad = nullptr;
}



void LoadState::LoadAssets()
{
	LoadShaders();
	LoadMaterials();
	LoadTextures();
	LoadModels();
	LoadAudioGroups();
}



void LoadState::LoadShaders()
{
	// Get the csv list of shader files we want to preload
	std::string shaderFilepaths = g_gameConfigBlackboard.GetValue("shaderFilesToLoad", "");
	if (!shaderFilepaths.empty())
	{
		// If we have shader files
		std::vector<std::string> splitFilepaths = ParseTokenStringToStrings(shaderFilepaths, ",");
		for (int i = 0; i < (int)splitFilepaths.size(); ++i)
		{
			// Load each shader file
			std::string shaderFilepath = RemoveAllWhitespace(splitFilepaths[i]);
			Shader::ParseShaders(shaderFilepath);
		}
	}
}



void LoadState::LoadMaterials()
{
	// Get the csv list of material files we want to preload
	std::string materialFilepaths = g_gameConfigBlackboard.GetValue("materialFilesToLoad", "");
	if (!materialFilepaths.empty())
	{
		// If we have material files
		std::vector<std::string> splitFilepaths = ParseTokenStringToStrings(materialFilepaths, ",");
		for (int i = 0; i < (int)splitFilepaths.size(); ++i)
		{
			// Load each material file
			std::string materialFilepath = RemoveAllWhitespace(splitFilepaths[i]);
			Material::ParseMaterials(materialFilepath);
		}
	}
}



void LoadState::LoadTextures()
{
	// Get the csv list of textures we want to preload
	std::string textureFilepaths = g_gameConfigBlackboard.GetValue("texturesToLoad", "");
	if (!textureFilepaths.empty())
	{
		// If we have textures
		std::vector<std::string> splitFilepaths = ParseTokenStringToStrings(textureFilepaths, ",");
		for (int i = 0; i < (int)splitFilepaths.size(); ++i)
		{
			// Load each texture
			std::string textureFilepath = RemoveAllWhitespace(splitFilepaths[i]);
			Texture::CreateOrGet(textureFilepath);
		}
	}
}



void LoadState::LoadModels()
{
	// Get the csv list of models we want to preload
	std::string modelFilepaths = g_gameConfigBlackboard.GetValue("modelsToLoad", "");
	if (!modelFilepaths.empty())
	{
		// If we have models
		std::vector<std::string> splitFilepaths = ParseTokenStringToStrings(modelFilepaths, ",");
		for (int i = 0; i < (int)splitFilepaths.size(); ++i)
		{
			// Load each model
			std::string modelFilepath = RemoveAllWhitespace(splitFilepaths[i]);
			Mesh::CreateOrGet(modelFilepath);
		}
	}
}



void LoadState::LoadAudioGroups()
{
	// Get the csv list of models we want to preload
	std::string soundGroupFilepaths = g_gameConfigBlackboard.GetValue("soundGroupFilesToLoad", "");
	if (!soundGroupFilepaths.empty())
	{
		// If we have models
		std::vector<std::string> splitFilepaths = ParseTokenStringToStrings(soundGroupFilepaths, ",");
		for (int i = 0; i < (int)splitFilepaths.size(); ++i)
		{
			// Load each model
			std::string soundGroupFilepath = RemoveAllWhitespace(splitFilepaths[i]);
			g_theAudio->LoadSoundGroups(soundGroupFilepath);
		}
	}
}



void LoadState::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	++m_frameNumber;
	if (m_frameNumber == 2)
	{
		LoadAssets();
		GetOwner()->StartTransitionToState(GAME_STATE_ATTRACT);
		GetOwner()->FinishTransition();
	}
}



void LoadState::Render2D() const
{
	g_theRenderer->BindCamera(m_camera);
	
	g_theRenderer->BindMaterial(m_loadScreenMaterial);
	g_theRenderer->DrawMesh(m_backgroundQuad->GetSubMesh(0));
}
