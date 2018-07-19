#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

#include "Engine/Core/Stopwatch.hpp"

#include "Engine/Core/Transform.hpp"

class RGBA;
class Texture;
class Sampler;
class Material;
class Camera;
class RenderScene;



enum eLightType
{
	LIGHT_TYPE_INVALID = -1,

	// Punctual
	LIGHT_TYPE_POINT = 0,
	LIGHT_TYPE_CONE,

	LIGHT_TYPE_DIRECTIONAL,		// DEPRECATED

	// Sun
	LIGHT_TYPE_SUN,

	// Area


	LIGHT_TYPE_COUNT
};



enum eShadowMapUpdateMode
{
	SHADOW_MAP_UPDATE_MODE_INVALID = -1,

	SHADOW_MAP_UPDATE_MODE_NEVER = 0,
	SHADOW_MAP_UPDATE_MODE_INTERVAL,
	SHADOW_MAP_UPDATE_MODE_ALWAYS,

	SHADOW_MAP_UPDATE_MODE_COUNT
};



// What gets sent to the shader
class LightData
{
public:
	void	 InitializePointLight(const Vector3& position, const RGBA& color, float intensity, const Vector3& attenuation = Vector3(0.0, 0.0f, 1.0f));
	void	 InitializeDirectionalLight(const Vector3& position, const Vector3& direction, const RGBA& color, float intensity, const Vector3& attenuation = Vector3(0.0, 0.0f, 1.0f));
	void	 InitializeConeLight(const Vector3& position, const Vector3& direction, float innerAngle, float outerAngle, const RGBA& color, float intensity, const Vector3& attenuation = Vector3(0.0, 0.0f, 1.0f));
	void	 InitializeSunLight(const Vector3& direction, const RGBA& color, float intensity);

	Vector3  GetPosition() const;
	void	 SetPosition(const Vector3& position);

	Vector3  GetDirection() const;
	void	 SetDirection(const Vector3& direction);

	RGBA	 GetColor() const;
	void	 SetColor(const RGBA& color);
			 
	float	 GetIntensity() const;
	void	 SetIntensity(float intensity);
			 
	Vector3	 GetAttenuation() const;
	void	 SetAttenuation(const Vector3& attenuation);
			 
	void	 Disable();
			 
	float	 GetIntensityAtPoint(const Vector3& point) const;



	Vector4  m_colorAndIntensity	  = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

	Vector3  m_lightDirection		  = Vector3(0.0f, 0.0f, 1.0f);
	float	 m_isPointLight 		  = 0.0f;							// 0 for yes, 1 for no

	Vector3  m_position				  = Vector3(0.0f, 0.0f, 0.0f);
	float	 m_spotLightInnerCosAngle = -2.0f;

	Vector3  m_attenuation			  = Vector3(0.0f, 0.0f, 1.0f);	
	float	 m_spotLightOuterCosAngle = -2.0f;  
};



class Light
{
public:
	// ----------------------------------------------------------------------------------------------------------------
	// -- Composition
	// ----------------------------------------------------------------------------------------------------------------
	Light();
	~Light();

	// Punctual Lights
	void					InitializePointLight(const Vector3& position, const RGBA& color, float intensity, const Vector3& attenuation = Vector3(0.0, 0.0f, 1.0f));
	void					InitializeConeLight(const Vector3& position, const Vector3& direction, float innerAngle, float outerAngle, const RGBA& color, float intensity, const Vector3& attenuation = Vector3(0.0, 0.0f, 1.0f));
	void					InitializeDirectionalLight(const Vector3& direction, const RGBA& color, float intensity); // Deprecated
	
	// Directional Light
	void					InitializeSunLight(const Vector3& direction, const RGBA& color, float intensity);	



	// ----------------------------------------------------------------------------------------------------------------
	// -- Light Info
	// ----------------------------------------------------------------------------------------------------------------
	// Light Type
	eLightType				GetLightType() const;

	// Position
	Vector3					GetPosition() const;
	void					SetPosition(const Vector3& position);

	// Direction
	Vector3					GetDirection() const;
	void					SetDirection(const Vector3& direction);
	
	// Color
	RGBA					GetColor() const;
	void					SetColor(const RGBA& color);
	
	// Intensity
	float					GetIntensity() const;
	void					SetIntensity(float intensity);
	float					GetIntensityAtPoint(const Vector3& point) const;
	
	// Attenuation
	Vector3					GetAttenuation() const;
	void					SetAttenuation(const Vector3& attenuation);
	
	// Disable
	void					Disable();

	// Raw Data
	const LightData&		GetLightData() const;



	// ----------------------------------------------------------------------------------------------------------------
	// -- Shadows
	// ----------------------------------------------------------------------------------------------------------------
	// Updating
	bool					ShouldUpdateShadowMaps() const;
	void					SetShadowMapUpdateMode(eShadowMapUpdateMode mode, float intervalInSeconds = (1.0f / 60.0f));
	void					UpdateShadowMaps(const RenderScene* scene, const Camera* camera = nullptr);

	// Shadow Casting
	bool					IsShadowCasting() const;
	void					SetShadowCastingState(bool state);

	// Cutoff Distance
	float					GetShadowMaxDistance() const;
	void					SetShadowMaxDistance(float distance = 100.0f);

	// Matrices
	const Matrix4			GetViewProjectionMatrix(int cascade = -1) const; // Leave default for spot/dir/point lights
	const Matrix4			GetProjectionMatrix(int cascade = -1) const;
	const Matrix4			GetLocalToWorldTransform(int cascade = -1) const;

	// Get Shadow Map
	const Texture*			GetShadowMapForDrawing(int cascade = -1) const;

	// DEPRECATED/ Directional Light
	void					SetDirectionalLightShadowDimensions(float xyDimensions);



	// ----------------------------------------------------------------------------------------------------------------
	// -- Statics
	// ----------------------------------------------------------------------------------------------------------------
	// Resolution
	static void				SetShadowMapResolution(int xyDimensions); // Shadow textures are square
	static int				GetShadowMapResolution();

	// Sampler for querying Shadow Maps
	static const Sampler*	GetShadowMapSampler();

	// Super slim pass through material for meshes when generating Shadow Maps
	static const Material*  GetShadowMapMaterial(); 

	// Default Shadow Map, pure white
	static Texture*		    GetDefaultShadowMap();

	// Initialization
	static void				TryAllocateDefaultShadowMap();
	static void				TryInitializeStatics();



private:
	void					UpdateTransformToMatchLightData();
	void					UpdateProjection();
	Texture*				CreateOrGetDepthTargetForUpdating(Texture** depthTarget);
						 

	// ----------------------------------------------------------------------------------------------------------------
	// -- Light Info
	// ----------------------------------------------------------------------------------------------------------------
	eLightType				m_lightType						= LIGHT_TYPE_INVALID;
	LightData				m_lightData;



	// ----------------------------------------------------------------------------------------------------------------
	// -- Shadows
	// ----------------------------------------------------------------------------------------------------------------
	// Updating
	bool					m_isShadowCasting				= true;
	eShadowMapUpdateMode	m_shadowMapUpdateMode			= SHADOW_MAP_UPDATE_MODE_ALWAYS;
	Stopwatch				m_shadowMapUpdateStopwatch;

	// Matrices
	Matrix4					m_projection;
	Transform				m_transform;									// For easy view matrix calculation
	
	// Matrices -> Cascades
	std::vector<Matrix4>	m_cascadeViewProjections;
	std::vector<Matrix4>	m_cascadeProjections;
	std::vector<Matrix4>	m_cascadeLocalToWorld;
	
	// Cutoff Distance
	float					m_distanceToFarPlane			= 100.0f;
	
	// Depth Targets
	Texture*				m_depthTarget_Forward			= nullptr;
	std::vector<Texture*>	m_cascadeDepthTargets;
	
	// DEPRECATED/ Directional Light
	float					m_directionalShadowDimensions	= 100.0f;



	// ----------------------------------------------------------------------------------------------------------------
	// -- Statics
	// ----------------------------------------------------------------------------------------------------------------
	static int				s_shadowMapResolution;
	static Texture*			s_defaultShadowMap;
	static Sampler*			s_shadowMapSampler;
	static Material*		s_shadowMappingMaterial;
};