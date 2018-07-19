#include "Engine/Rendering/Light.hpp"

#include <vector>

#include "Engine/Core/EngineCommon.h"
#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/RGBA.hpp"
#include "Engine/Core/Clock.hpp"

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Sampler.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/RenderScene.hpp"
#include "Engine/Rendering/Renderable.hpp"

extern Clock*		g_theMasterClock;
extern Renderer*	g_theRenderer;



//-------------------------------------------------------------------------------------------------------
Vector4 PackColorAndIntensity(const RGBA& color, float brightness)
{
	float r,g,b,a;
	color.GetAsFloats(r, g, b, a);

	Vector4 c = Vector4(r,g,b, brightness); 
	return c;
}


//-------------------------------------------------------------------------------------------------------
void LightData::InitializePointLight(const Vector3& position, const RGBA& color, float intensity, const Vector3& attenuation)
{
	m_colorAndIntensity = PackColorAndIntensity(color, intensity);

	m_lightDirection = Vector3(0.0f, 0.0f, 1.0f);
	m_isPointLight = 0.0f;

	m_position = position;
	m_spotLightInnerCosAngle = -2.0f;

	m_attenuation = attenuation;
	m_spotLightOuterCosAngle = -2.0f;
}


//-------------------------------------------------------------------------------------------------------
void LightData::InitializeDirectionalLight(const Vector3& position, const Vector3& direction, const RGBA& color, float intensity, const Vector3& attenuation)
{
	m_colorAndIntensity = PackColorAndIntensity(color, intensity);

	m_lightDirection = direction.GetNormalized();
	m_isPointLight = 1.0f;

	m_position = position;
	m_spotLightInnerCosAngle = -2.0f;

	m_attenuation = attenuation;
	m_spotLightOuterCosAngle = -2.0f;
}


//-------------------------------------------------------------------------------------------------------
void LightData::InitializeConeLight(const Vector3& position, const Vector3& direction, float innerAngleDegrees, float outerAngleDegrees, const RGBA& color, float intensity, const Vector3& attenuation)
{
	m_colorAndIntensity = PackColorAndIntensity(color, intensity);

	m_lightDirection = direction.GetNormalized();
	m_isPointLight = 1.0f;

	m_position = position;
	m_spotLightInnerCosAngle =  CosDegrees(innerAngleDegrees);

	m_attenuation = attenuation;
	m_spotLightOuterCosAngle = CosDegrees(outerAngleDegrees);
}


//-------------------------------------------------------------------------------------------------------
void LightData::InitializeSunLight(const Vector3& direction, const RGBA& color, float intensity)
{
	m_colorAndIntensity = PackColorAndIntensity(color, intensity);

	m_lightDirection = direction.GetNormalized();
	m_isPointLight = 1.0f;

	m_position = Vector3::ZEROS;
	m_spotLightInnerCosAngle = -2.0f;

	m_attenuation = Vector3(1.0f, 0.0f, 0.0f); // No fall off
	m_spotLightOuterCosAngle = -2.0f;
}


//-------------------------------------------------------------------------------------------------------
Vector3 LightData::GetPosition() const
{
	return m_position;
}


//-------------------------------------------------------------------------------------------------------
void LightData::SetPosition(const Vector3& position)
{
	m_position = position;
}


//-------------------------------------------------------------------------------------------------------
Vector3 LightData::GetDirection() const
{
	return m_lightDirection;
}


//-------------------------------------------------------------------------------------------------------
void LightData::SetDirection(const Vector3& direction)
{
	m_lightDirection = direction;
}


//-------------------------------------------------------------------------------------------------------
RGBA LightData::GetColor() const
{
	RGBA color;

	color.SetAsFloats(m_colorAndIntensity.x, m_colorAndIntensity.y, m_colorAndIntensity.z);

	return color;
}


//-------------------------------------------------------------------------------------------------------
void LightData::SetColor(const RGBA& color)
{
	Vector4 glCompatibleColor = color.GetAsVector4();
	m_colorAndIntensity.x = glCompatibleColor.x;
	m_colorAndIntensity.y = glCompatibleColor.y;
	m_colorAndIntensity.z = glCompatibleColor.z;
}


//-------------------------------------------------------------------------------------------------------
float LightData::GetIntensity() const
{
	return m_colorAndIntensity.w;
}


//-------------------------------------------------------------------------------------------------------
void LightData::SetIntensity(float intensity)
{
	m_colorAndIntensity.w = intensity;
}


//-------------------------------------------------------------------------------------------------------
Vector3	LightData::GetAttenuation() const
{
	return m_attenuation;
}


//-------------------------------------------------------------------------------------------------------
void LightData::SetAttenuation(const Vector3& attenuation)
{
	m_attenuation = attenuation;
}


//-------------------------------------------------------------------------------------------------------
void LightData::Disable()
{
	m_colorAndIntensity.w = 0.0f;
}


//-------------------------------------------------------------------------------------------------------
float LightData::GetIntensityAtPoint(const Vector3& point) const
{
	Vector3 directionToLight = m_position - point;
	float distanceToLight = directionToLight.NormalizeAndGetLength();
	float attenuation = DotProduct(m_attenuation, Vector3(1.0f, distanceToLight, distanceToLight * distanceToLight));

	float intensity = 0.0f;
	if (attenuation != 0.0f)
	{
		intensity = m_colorAndIntensity.w / attenuation;
	}
	return intensity;
}



// -----------------------------------------------------------------------------------------------------------------------
// LIGHT CLASS
// -----------------------------------------------------------------------------------------------------------------------
Light::Light()
	: m_shadowMapUpdateStopwatch(g_theMasterClock)
{
	TryInitializeStatics();


	// Shadow mapping data
	m_shadowMapUpdateStopwatch.SetTimer(1.0f / 60.0f);
}


//-------------------------------------------------------------------------------------------------------
Light::~Light()
{

}


//-------------------------------------------------------------------------------------------------------
void Light::InitializePointLight(const Vector3& position, const RGBA& color, float intensity, const Vector3& attenuation)
{
	m_lightType = LIGHT_TYPE_POINT;
	m_lightData.InitializePointLight(position, color, intensity, attenuation);

	m_transform.SetLocalOrientation(Quaternion()); // Realign the transform to the XYZ world axes, only useful if we are changing light types
	UpdateTransformToMatchLightData();
	UpdateProjection();
}


//-------------------------------------------------------------------------------------------------------
void Light::InitializeDirectionalLight(const Vector3& direction, const RGBA& color, float intensity)
{
	m_lightType = LIGHT_TYPE_DIRECTIONAL;
	m_lightData.InitializeDirectionalLight(Vector3::ZEROS, direction, color, intensity, Vector3(1.0f, 0.0f, 0.0f));
	SetShadowCastingState(false);

	UpdateTransformToMatchLightData();
	UpdateProjection();
}


//-------------------------------------------------------------------------------------------------------
void Light::InitializeConeLight(const Vector3& position, const Vector3& direction, float innerAngle, float outerAngle, const RGBA& color, float intensity, const Vector3& attenuation)
{
	m_lightType = LIGHT_TYPE_CONE;
	m_lightData.InitializeConeLight(position, direction, innerAngle, outerAngle, color, intensity, attenuation);

	UpdateTransformToMatchLightData();
	UpdateProjection();
}


//-------------------------------------------------------------------------------------------------------
void Light::InitializeSunLight(const Vector3& direction, const RGBA& color, float intensity)
{
	m_lightType = LIGHT_TYPE_SUN;
	m_lightData.InitializeSunLight(direction, color, intensity);

	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		m_cascadeViewProjections.push_back(Matrix4());
		m_cascadeProjections.push_back(Matrix4());
		m_cascadeLocalToWorld.push_back(Matrix4());
	}

	// Generate initial cascade depth targets
	while((int)m_cascadeDepthTargets.size() < NUM_CASCADES)
	{
		m_cascadeDepthTargets.push_back(nullptr);
	}
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		CreateOrGetDepthTargetForUpdating(&m_cascadeDepthTargets[i]);
	}

	UpdateTransformToMatchLightData();
}


//-------------------------------------------------------------------------------------------------------
eLightType Light::GetLightType() const
{
	return m_lightType;
}


//-------------------------------------------------------------------------------------------------------
Vector3	Light::GetPosition() const
{
	return m_lightData.GetPosition();
}


//-------------------------------------------------------------------------------------------------------
void Light::SetPosition(const Vector3& position)
{
	m_lightData.SetPosition(position);
	UpdateTransformToMatchLightData();
}


//-------------------------------------------------------------------------------------------------------
Vector3 Light::GetDirection() const
{
	return m_lightData.GetDirection();
}


//-------------------------------------------------------------------------------------------------------
void Light::SetDirection(const Vector3& direction)
{
	m_lightData.SetDirection(direction);
	UpdateTransformToMatchLightData();
}


//-------------------------------------------------------------------------------------------------------
RGBA Light::GetColor() const
{
	return m_lightData.GetColor();
}


//-------------------------------------------------------------------------------------------------------
void Light::SetColor(const RGBA& color)
{
	m_lightData.SetColor(color);
}


//-------------------------------------------------------------------------------------------------------
float Light::GetIntensity() const
{
	return m_lightData.GetIntensity();
}


//-------------------------------------------------------------------------------------------------------
void Light::SetIntensity(float intensity)
{
	m_lightData.SetIntensity(intensity);
}


//-------------------------------------------------------------------------------------------------------
Vector3	Light::GetAttenuation() const
{
	return m_lightData.GetAttenuation();
}


//-------------------------------------------------------------------------------------------------------
void Light::SetAttenuation(const Vector3& attenuation)
{
	m_lightData.SetAttenuation(attenuation);
}


//-------------------------------------------------------------------------------------------------------
void Light::Disable()
{
	m_lightData.Disable();
}


//-------------------------------------------------------------------------------------------------------
float Light::GetIntensityAtPoint(const Vector3& point) const
{
	return m_lightData.GetIntensityAtPoint(point);
}


//-------------------------------------------------------------------------------------------------------
const LightData& Light::GetLightData() const
{
	return m_lightData;
}


//-------------------------------------------------------------------------------------------------------
bool Light::ShouldUpdateShadowMaps() const
{
	TODO("If the shadow map size has changed on us we should forcibly update");

	bool shouldUpdate = false;

	if (IsShadowCasting())
	{
		switch(m_shadowMapUpdateMode)
		{
		case SHADOW_MAP_UPDATE_MODE_NEVER:
		{
			// Nope
			break;
		}
		case SHADOW_MAP_UPDATE_MODE_INTERVAL:
		{
			// Maybe?
			bool canBeDecremented = m_shadowMapUpdateStopwatch.HasElapsed();
			if (canBeDecremented)
			{
				// If we have passed our update interval we should update
				shouldUpdate = true;
			}
			break;
		}
		case SHADOW_MAP_UPDATE_MODE_ALWAYS:
		{
			// Yup
			shouldUpdate = true;
			break;
		}
		case SHADOW_MAP_UPDATE_MODE_INVALID: // Fall through
		case SHADOW_MAP_UPDATE_MODE_COUNT:
		default:
		{
			// Never come here.
			GUARANTEE_OR_DIE(false, "m_shadowMapUpdateMode is an invalid value.");
			break;
		}
		}
	}

	return shouldUpdate;
}



void Light::SetShadowMapUpdateMode(eShadowMapUpdateMode mode, float intervalInSeconds)
{
	switch(mode)
	{
	case SHADOW_MAP_UPDATE_MODE_INTERVAL:
	{
		m_shadowMapUpdateMode = mode;
		
		// Update the timer
		m_shadowMapUpdateStopwatch.Reset();
		m_shadowMapUpdateStopwatch.SetTimer(intervalInSeconds);
		break;
	}
	case SHADOW_MAP_UPDATE_MODE_NEVER: // Fall through
	case SHADOW_MAP_UPDATE_MODE_ALWAYS: 
	{
		m_shadowMapUpdateMode = mode;
		break;
	}
	case SHADOW_MAP_UPDATE_MODE_INVALID: // Fall through
	case SHADOW_MAP_UPDATE_MODE_COUNT:
	default:
	{
		GUARANTEE_OR_DIE(false, "ShadowMapUpdateMode should never be passed, INVALID or COUNT");
		break;
	}
	}
}


//-------------------------------------------------------------------------------------------------------
void Light::UpdateShadowMaps(const RenderScene* scene, const Camera* camera)
{
	Camera shadowMapCamera;

	if (m_shadowMapUpdateMode == SHADOW_MAP_UPDATE_MODE_INTERVAL)
	{
		m_shadowMapUpdateStopwatch.DecrementAll();
	}

	TODO("Verify textures are the correct size");

	switch(GetLightType())
	{
	case LIGHT_TYPE_POINT:
	{
		GUARANTEE_OR_DIE(false, "Point Light shadows are currently unsupported");
		break;
	}
	case LIGHT_TYPE_CONE:
	case LIGHT_TYPE_DIRECTIONAL:
	{
		// Set the camera up to render to a texture the size of our depth target
		int shadowMapResolution = GetShadowMapResolution();
		shadowMapCamera.OverrideViewportBaseResolution(true, IntVector2(shadowMapResolution, shadowMapResolution));

		// Bind our depth target to the camera
		shadowMapCamera.SetDepthStencilTarget(CreateOrGetDepthTargetForUpdating(&m_depthTarget_Forward));
		

		// Set up the camera up to mirror the light
		shadowMapCamera.SetLocalToParentMatrix(m_transform.GetLocalToWorldMatrix());
		shadowMapCamera.SetProjection(m_projection);

		g_theRenderer->BindCamera(&shadowMapCamera);
		g_theRenderer->ClearDepth();
		

		// Render all shadow casting objects in the scene
		g_theRenderer->BindMaterial(GetShadowMapMaterial());
		const std::vector<Renderable*>& renderables = scene->GetRenderables();
		for (int i = 0; i < (int)renderables.size(); ++i)
		{
			Renderable* renderable = renderables[i];
			if (renderable->CastsShadow())
			{
				const Mesh* renderablesMesh = renderable->GetMesh();
				for (int j = 0; j < (int)renderablesMesh->GetSubMeshCount(); ++j)
				{
					if (renderable->GetMaterial(j)->GetShader()->GetRenderQueue() == RENDER_QUEUE_OPAQUE)
					{
						g_theRenderer->BindModelMatrix(renderable->GetModelMatrix());
						g_theRenderer->DrawMesh(renderablesMesh->GetSubMesh(j));
					}
				}
			}
		}


		break;
	}
	case LIGHT_TYPE_SUN:
	{
		m_cascadeViewProjections.clear();
		m_cascadeProjections.clear();
		m_cascadeLocalToWorld.clear();

		// Correctly size the cascade depth targets
		while((int)m_cascadeDepthTargets.size() < NUM_CASCADES)
		{
			m_cascadeDepthTargets.push_back(nullptr);
		}

		std::vector<Matrix4>  cascadeProjections;
		std::vector<Matrix4>  cascadeCameraBasisMatrixs;
		
		for (int i = 0; i < NUM_CASCADES; ++i)
		{
			Matrix4 frustum = camera->GetProjectionForCascade(i);
			Matrix4 cameraLocalToWorld = camera->GetLocalToWorldMatrix();

			std::vector<Vector3> worldSpaceFrustumPoints = GetPointsForProjection(frustum); // This starts in camera space
			for (int p = 0; p < (int)worldSpaceFrustumPoints.size(); ++p)
			{
				worldSpaceFrustumPoints[p] = (cameraLocalToWorld * Vector4(worldSpaceFrustumPoints[p], 1.0f)).XYZ();
			}



			// Projection Bounds Generation------------------------------------------------------------------------------------------
			//-----------------------------------------------------------------------------------------------------------------------
			Matrix4 cascadeLocalToWorld = Matrix4::CreateLookAt(Vector3::ZEROS, GetDirection());
			Matrix4 cascadeWorldToLocal = cascadeLocalToWorld;
			cascadeWorldToLocal.Invert();

			AABB3 bounds;
			std::vector<Vector3> cascadeSpaceFrustumPoints;
			for (int p = 0; p < (int)worldSpaceFrustumPoints.size(); ++p)
			{
				Vector3 cascadeSpaceFrustumPoint = (cascadeWorldToLocal * Vector4(worldSpaceFrustumPoints[p], 1.0f)).XYZ();
				bounds.StretchToIncludePoint(cascadeSpaceFrustumPoint);

				cascadeSpaceFrustumPoints.push_back(cascadeSpaceFrustumPoint);
			}
			Vector3 cascadeSpaceFrustumCenter = bounds.GetCenter();
			Vector3 cascadeSpaceHalfDimensions = bounds.GetHalfDimensions();

			// Frustum Radius, uses inscribed trapezoid method
			float a = (cascadeSpaceFrustumPoints[7] - cascadeSpaceFrustumPoints[6]).GetLength();
			float b = (cascadeSpaceFrustumPoints[3] - cascadeSpaceFrustumPoints[2]).GetLength();
			float c = (cascadeSpaceFrustumPoints[7] - cascadeSpaceFrustumPoints[3]).GetLength();
			float frustumRadius_Trapezoid = c * SQRT( ((a * b) + (c * c)) / ((4 * c * c) - ( (a - b) * (a - b) )) );

			Vector3 worldSpaceFrustumCenter = (cascadeLocalToWorld * Vector4(cascadeSpaceFrustumCenter, 1.0f)).XYZ();
			float frustumEncompassingRadius = frustumRadius_Trapezoid;
			Vector3 frustumDimensions = Vector3(frustumEncompassingRadius, frustumEncompassingRadius, frustumEncompassingRadius);
			
			// DEBUG 
			Matrix4 camProj = Matrix4::CreateOrthographicProjection(-frustumEncompassingRadius, frustumEncompassingRadius, -frustumEncompassingRadius, frustumEncompassingRadius, -frustumEncompassingRadius * 6.0f, frustumEncompassingRadius * 6.0f);
			Vector3 wsCamCenter = worldSpaceFrustumCenter - (GetDirection() * cascadeSpaceHalfDimensions.z * 2.0f);
			Matrix4 cLtoW = Matrix4::CreateLookAt(wsCamCenter, worldSpaceFrustumCenter);
			//-----------------------------------------------------------------------------------------------------------------------
			//-----------------------------------------------------------------------------------------------------------------------



			// Texel Snapping --------------------------------------------------------------------------------------------------------
			// -----------------------------------------------------------------------------------------------------------------------
			// Create texel space basis
			float texelsPerWoldUnit = GetShadowMapResolution() / (frustumEncompassingRadius * 2.0f);

			Matrix4 lookAt = Matrix4::CreateLookAt(Vector3::ZEROS, GetDirection());
			Matrix4 texelScale = Matrix4::CreateUniformScale(texelsPerWoldUnit);
			Matrix4 texelSpaceBasis = texelScale * lookAt;

			Matrix4 texelSpaceBasisInverse = texelSpaceBasis;
			texelSpaceBasisInverse.Invert();

			// Transform our frustum center into texel space, snap it to the nearest texels, transform back to camera space
			Vector3 frustumCenter = (texelSpaceBasis * Vector4(worldSpaceFrustumCenter, 1.0f)).XYZ();
			frustumCenter.x = Floor(frustumCenter.x);
			frustumCenter.y = Floor(frustumCenter.y);
			frustumCenter.z = Floor(frustumCenter.z);
			worldSpaceFrustumCenter = (texelSpaceBasisInverse * Vector4(frustumCenter, 1.0f)).XYZ();
			// -----------------------------------------------------------------------------------------------------------------------
			// -----------------------------------------------------------------------------------------------------------------------



			// Create the view matrix for this cascade
			Vector3 worldSpaceCameraCenter = worldSpaceFrustumCenter - (GetDirection() * frustumEncompassingRadius * 2.0f);
			Matrix4 cameraBasis = Matrix4::CreateLookAt(worldSpaceCameraCenter, worldSpaceFrustumCenter);
			cascadeCameraBasisMatrixs.push_back(cameraBasis);

			// Create the projection matrix for this cascade
			// near and far plane are punched out a bit to make sure we get everything
			Matrix4 cascadeProjection = Matrix4::CreateOrthographicProjection(-frustumEncompassingRadius, frustumEncompassingRadius, -frustumEncompassingRadius, frustumEncompassingRadius, -frustumEncompassingRadius * 6.0f, frustumEncompassingRadius * 6.0f);
			cascadeProjections.push_back(cascadeProjection);
		}


		// SAME FOR EACH CASCADE
		// Set the camera up to render to a texture the size of our depth target
		int shadowMapResolution = GetShadowMapResolution();
		shadowMapCamera.OverrideViewportBaseResolution(true, IntVector2(shadowMapResolution, shadowMapResolution));

		for (int i = 0; i < NUM_CASCADES; ++i)
		{

			// DEPTH TARGET, PROJECTION, POSITION CHANGE FOR EACH CASCADE
			// Bind our depth target to the camera
			shadowMapCamera.SetDepthStencilTarget(CreateOrGetDepthTargetForUpdating(&m_cascadeDepthTargets[i]));


			// Set up the camera up to mirror the light
			shadowMapCamera.SetLocalToParentMatrix(cascadeCameraBasisMatrixs[i]);
			shadowMapCamera.SetProjection(cascadeProjections[i]);

			// Save off the Cascades View Projection matrix
			m_cascadeViewProjections.push_back(shadowMapCamera.GetProjectionMatrix() * shadowMapCamera.GetWorldToLocalMatrix());
			m_cascadeProjections.push_back(shadowMapCamera.GetProjectionMatrix());
			m_cascadeLocalToWorld.push_back(shadowMapCamera.GetLocalToWorldMatrix());
			//if (i == 1)
			//{
				//DebugDraw_Frustum(0.0f, shadowMapCamera.GetProjectionMatrix(), shadowMapCamera.GetLocalToWorldMatrix(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//}

			g_theRenderer->BindCamera(&shadowMapCamera);
			g_theRenderer->ClearDepth();


			// Render all shadow casting objects in the scene
			g_theRenderer->BindMaterial(GetShadowMapMaterial());
			const std::vector<Renderable*>& renderables = scene->GetRenderables();
			for (int r = 0; r < (int)renderables.size(); ++r)
			{
				Renderable* renderable = renderables[r];
				if (renderable->CastsShadow())
				{
					const Mesh* renderablesMesh = renderable->GetMesh();
					for (int j = 0; j < (int)renderablesMesh->GetSubMeshCount(); ++j)
					{
						if (renderable->GetMaterial(j)->GetShader()->GetRenderQueue() == RENDER_QUEUE_OPAQUE)
						{
							g_theRenderer->BindModelMatrix(renderable->GetModelMatrix());
							g_theRenderer->DrawMesh(renderablesMesh->GetSubMesh(j));
						}
					}
				}
			}
		}


		break;
	}
	case LIGHT_TYPE_INVALID: // Fall through
	case LIGHT_TYPE_COUNT:
	default:
	{
		GUARANTEE_OR_DIE(false, "Should never get here");
		break;
	}
	}

}


//-------------------------------------------------------------------------------------------------------
bool Light::IsShadowCasting() const
{
	return m_isShadowCasting;
}


//-------------------------------------------------------------------------------------------------------
void Light::SetShadowCastingState(bool state)
{
	m_isShadowCasting = state;

	// If we are no longer a shadow casting light reset its depthTargets to all white so shadow checks always pass
	if (!IsShadowCasting())
	{
		delete m_depthTarget_Forward;
		m_depthTarget_Forward = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------------
float Light::GetShadowMaxDistance() const
{
	return m_distanceToFarPlane;
}


//-------------------------------------------------------------------------------------------------------
void Light::SetShadowMaxDistance(float distance)
{
	m_distanceToFarPlane = distance;
	UpdateProjection();
}


//-------------------------------------------------------------------------------------------------------
const Texture* Light::GetShadowMapForDrawing(int cascade) const
{
	Texture* shadowMap = nullptr;

	switch(GetLightType())
	{
	case LIGHT_TYPE_POINT:
	case LIGHT_TYPE_DIRECTIONAL:
	case LIGHT_TYPE_CONE:
	{
		// Punctual
		if (m_depthTarget_Forward != nullptr)
		{
			// If we have a shadow map use it
			shadowMap = m_depthTarget_Forward;
		}
		else
		{
			// Else use the default shadow map
			shadowMap = GetDefaultShadowMap();
		}

		break;
	}
	case LIGHT_TYPE_SUN:
	{
		// Sun
		GUARANTEE_OR_DIE(cascade < (int)m_cascadeDepthTargets.size() && cascade >= 0, "Requested cascade is out of bounds");

		shadowMap = m_cascadeDepthTargets[cascade];

		break;
	}
	case LIGHT_TYPE_INVALID:
	case LIGHT_TYPE_COUNT:
	default:
		break;
	}

	return shadowMap;
}


//-------------------------------------------------------------------------------------------------------
const Matrix4 Light::GetProjectionMatrix(int cascade) const
{
	if (cascade < 0)
	{
		return m_projection;
	}
	else
	{
		return m_cascadeProjections[cascade];
	}
}


//-------------------------------------------------------------------------------------------------------
const Matrix4 Light::GetLocalToWorldTransform(int cascade) const
{
	if (cascade < 0)
	{
		// For non cascaded lights
		return m_transform.GetLocalToWorldMatrix();
	}
	else
	{
		return m_cascadeLocalToWorld[cascade];
	}
}


//-------------------------------------------------------------------------------------------------------
const Matrix4 Light::GetViewProjectionMatrix(int cascade) const
{
	Matrix4 viewProjectionMatrix;
	if (cascade == -1)
	{
		viewProjectionMatrix = m_projection * m_transform.GetWorldToLocalMatrix();
	}
	else
	{
		GUARANTEE_OR_DIE(cascade >= 0 && cascade < NUM_CASCADES, "Cascade is out of bounds");

		viewProjectionMatrix = m_cascadeViewProjections[cascade];
	}
	return viewProjectionMatrix;
}


//-------------------------------------------------------------------------------------------------------
void Light::SetDirectionalLightShadowDimensions(float xyDimensions)
{
	GUARANTEE_OR_DIE(xyDimensions != 0.0f, "Directional Light shadow dimensions should not be 0.0f");

	m_directionalShadowDimensions = xyDimensions;
	UpdateProjection();
}


//-------------------------------------------------------------------------------------------------------
void Light::SetShadowMapResolution(int xyDimensions)
{
	// Short Circuit
	if (xyDimensions == s_shadowMapResolution)
	{
		return;
	}

	s_shadowMapResolution = xyDimensions;

	// If we changed the shadow map resolution me need to update the default shadow map
	TryAllocateDefaultShadowMap();

}


//-------------------------------------------------------------------------------------------------------
int Light::GetShadowMapResolution()
{
	return s_shadowMapResolution;
}


//-------------------------------------------------------------------------------------------------------
const Sampler* Light::GetShadowMapSampler()
{
	if (s_shadowMapSampler == nullptr)
	{
		s_shadowMapSampler = new Sampler();
		s_shadowMapSampler->Initialize(true, false, Sampler::WRAP_MODE_CLAMP_TO_BORDER, RGBA());
	}

	return s_shadowMapSampler;
}


//-------------------------------------------------------------------------------------------------------
const Material* Light::GetShadowMapMaterial()
{
	// If the material hasn't been retrieved yet, go get it
	if (s_shadowMappingMaterial == nullptr)
	{
		s_shadowMappingMaterial = Material::Get("ShadowMapping");
	}

	return s_shadowMappingMaterial;
}


//-------------------------------------------------------------------------------------------------------
Texture* Light::GetDefaultShadowMap()
{
	TryAllocateDefaultShadowMap();
	return s_defaultShadowMap;
}


//-------------------------------------------------------------------------------------------------------
void Light::TryAllocateDefaultShadowMap()
{
	int shadowMapResolution = GetShadowMapResolution();
	if (s_defaultShadowMap == nullptr)
	{
		// If we have no default shadow map, create one
		s_defaultShadowMap = Texture::CreateRenderTarget(shadowMapResolution, shadowMapResolution, Texture::TEXTURE_FORMAT_D24S8);
		Camera camera;
		camera.SetDepthStencilTarget(s_defaultShadowMap);
		camera.OverrideViewportBaseResolution(true, IntVector2(shadowMapResolution, shadowMapResolution));
		g_theRenderer->BindCamera(&camera);
		g_theRenderer->ClearDepth();
	}
	else if (s_defaultShadowMap->GetDimensions().x != shadowMapResolution)
	{
		// If we changed the shadow maps resolution, create a new one
		delete s_defaultShadowMap;
		s_defaultShadowMap = Texture::CreateRenderTarget(shadowMapResolution, shadowMapResolution, Texture::TEXTURE_FORMAT_D24S8);
		Camera camera;
		camera.SetDepthStencilTarget(s_defaultShadowMap);
		camera.OverrideViewportBaseResolution(true, IntVector2(shadowMapResolution, shadowMapResolution));
		g_theRenderer->BindCamera(&camera);
		g_theRenderer->ClearDepth();
	}
}


//-------------------------------------------------------------------------------------------------------
void Light::TryInitializeStatics()
{
	GetDefaultShadowMap();
	GetShadowMapMaterial();
	GetShadowMapSampler();
	
}



//-------------------------------------------------------------------------------------------------------
void Light::UpdateTransformToMatchLightData()
{
	// Sync the transform up with the master light data
	m_transform.SetWorldPosition(m_lightData.GetPosition());

	// Orient the light if it is a directional light or a cone light, but leave it axis aligned if it is a point light.
	if (m_lightType == LIGHT_TYPE_CONE || m_lightType == LIGHT_TYPE_DIRECTIONAL)
	{
		m_transform.SetLocalOrientation(Quaternion::LookAt(m_lightData.GetDirection()));
	}
}



void Light::UpdateProjection()
{
	switch(m_lightType)
	{
	case LIGHT_TYPE_POINT:
	{
		break;
	}
	case LIGHT_TYPE_CONE:
	{
		float outerAngleDegrees = ACosDegrees(m_lightData.m_spotLightOuterCosAngle) + 5.0f;
		m_projection = Matrix4::CreatePerspectiveProjection(outerAngleDegrees, 1.0f, 1.0f, GetShadowMaxDistance());
		break;
	}
	case LIGHT_TYPE_DIRECTIONAL:
	{
		float projectionHalfDimensions = ((float)GetShadowMapResolution() / m_directionalShadowDimensions) * 0.5f;
		m_projection = Matrix4::CreateOrthographicProjection(-projectionHalfDimensions, projectionHalfDimensions, -projectionHalfDimensions, projectionHalfDimensions, 0.0f, GetShadowMaxDistance());
		break;
	}
	case LIGHT_TYPE_INVALID:
	case LIGHT_TYPE_COUNT:
	default:
	{
		GUARANTEE_OR_DIE(false, "Light should never get here");
		break;
	}
	}
}


//-------------------------------------------------------------------------------------------------------
Texture* Light::CreateOrGetDepthTargetForUpdating(Texture** testDepthTarget)
{
	Texture* depthTarget = nullptr;

	if (IsShadowCasting())
	{
		if (*testDepthTarget == nullptr)
		{
			// If we don't have a depth target but we want to cast shadows create a new one
			int shadowMapResolution = GetShadowMapResolution();
			*testDepthTarget = Texture::CreateRenderTarget(shadowMapResolution, shadowMapResolution, Texture::TEXTURE_FORMAT_D24S8);
			depthTarget = *testDepthTarget;
		}
		else
		{
			// We already have a depth target check to see if its the correct size
			int shadowMapResolution = GetShadowMapResolution();
			IntVector2 depthTargetResolution = (*testDepthTarget)->GetDimensions();
			if (depthTargetResolution.x == shadowMapResolution && depthTargetResolution.y == shadowMapResolution)
			{
				// We can draw to our current depth target with no problems
				depthTarget = *testDepthTarget;
			}
			else
			{
				// Our depth target is not the correct size, so we need to make a new one
				delete *testDepthTarget;
				*testDepthTarget = nullptr;

				*testDepthTarget = Texture::CreateRenderTarget(shadowMapResolution, shadowMapResolution, Texture::TEXTURE_FORMAT_D24S8);
				depthTarget = *testDepthTarget;
			}
		}
	}

	return depthTarget;
}


//-------------------------------------------------------------------------------------------------------
int			Light::s_shadowMapResolution = /*4096*/2048;
Texture*	Light::s_defaultShadowMap = nullptr;
Sampler*	Light::s_shadowMapSampler = nullptr;
Material*   Light::s_shadowMappingMaterial = nullptr;