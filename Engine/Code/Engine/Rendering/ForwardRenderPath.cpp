#include "Engine/Rendering/ForwardRenderPath.hpp"

#include <algorithm>

#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Profiler/ProfileScope.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/MeshUtils.hpp"

#include "Engine/Rendering/RenderScene.hpp"
#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/Light.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/ParticleEmitter.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Rendering/Renderer.hpp"
extern Renderer* g_theRenderer;



ForwardRenderPath::ForwardRenderPath()
{
	m_skyboxMesh = CreateCube(Vector3::ZEROS, Vector3::ONES);
	m_skyboxShader = Shader::Get("Skybox");	
}



bool CameraSortFunction(const Camera* lhs, const Camera* rhs)
{
	bool isLHSLess = false;

	if (lhs->GetCameraOrder() < rhs->GetCameraOrder())
	{
		isLHSLess = true;
	}

	return isLHSLess;
}



void SortCameras(std::vector<Camera*>& cameras)
{
	std::sort(cameras.begin(), cameras.end(), CameraSortFunction);
}



void ForwardRenderPath::Render(RenderScene* scene)
{
	PROFILE_SCOPE_FUNCTION();

	// Update fog
	scene->UpdateFog();

	Profiler_Push("ForwardRenderPath::Render.UpdatePunctualShadowMaps");
	// Update the shadow maps for all the lights in the scene
	const std::vector<Light*> lights = scene->GetLights();
	for (int i = 0; i < (int)lights.size(); ++i)
	{
		Light* light = lights[i];
		if (light->IsShadowCasting() && light->ShouldUpdateShadowMaps())
		{
			light->UpdateShadowMaps(scene);
		}
	}
	Profiler_Pop();

	// Render the scene for each camera
	std::vector<Camera*>& cameras = scene->GetMutableCameras();
	SortCameras(cameras);

	for (int i = 0; i < (int)cameras.size(); ++i)
	{
		// Update the sun if the scene has one. This is dependent on a camera so it cannot be updated prior to this
		Profiler_Push("ForwardRenderPath::Render.UpdateCascadingShadowMaps");
		if (scene->HasSun() && cameras[i]->DrawSun())
		{
			scene->GetMutableSun()->UpdateShadowMaps(scene, cameras[i]);
			g_theRenderer->SetSunLight(scene->GetMutableSun()); 
		}
		Profiler_Pop();

		RenderForCamera(cameras[i], scene);
	}
}


#include "Engine/Rendering/DebugRender.hpp"
#include "Engine/Rendering/Texture.hpp"
void ForwardRenderPath::RenderForCamera(Camera* camera, RenderScene* scene)
{
	PROFILE_SCOPE_FUNCTION();

	g_theRenderer->BindCamera(camera);
	ClearForCamera(camera);

	g_theRenderer->SetShadowCascadeBreakpoints(camera);


	// Generate particles for this camera view
	const std::vector<ParticleEmitter*> emitters = scene->GetParticleEmitters();
	for (int i = 0; i < (int)emitters.size(); ++i)
	{
		emitters[i]->UpdateRenderableForCamera(camera);
	}


	const std::vector<Renderable*>& renderables = scene->GetRenderables();

	std::vector<DrawCall> drawCalls;
	drawCalls.reserve(renderables.size());


	// Generate draw calls for scene
	Profiler_Push("ForwardRenderPath::RenderForCamera.GenerateDrawCalls");
	for (int i = 0; i < (int)renderables.size(); ++i)
	{
		Renderable* renderable = renderables[i];
		const Mesh* mesh = renderable->GetMesh();
		
		for (int subMeshIndex = 0; subMeshIndex < (int)mesh->GetSubMeshCount(); ++subMeshIndex)
		{
			DrawCall drawCall;
			drawCall.m_model			= renderable->GetModelMatrix();
			drawCall.m_mesh				= mesh->GetSubMesh(subMeshIndex);
			drawCall.m_material			= renderable->GetMaterial(subMeshIndex);
			drawCall.m_layer			= drawCall.m_material->GetShader()->GetRenderLayer();
			drawCall.m_queue			= drawCall.m_material->GetShader()->GetRenderQueue();
			drawCall.m_receivesShadows	= renderable->ReceivesShadows();

			// If is a lit material find the lights affecting it
			if (renderable->GetMaterial()->GetShader()->IsLit()) 
			{
				ComputeMostContributingLights( drawCall, scene ); 
			}

			drawCalls.push_back(drawCall); 
		}
	}


	// Generate draw calls for temporary renderables
	for (int i = 0; i < (int)m_temporaryRendeables.size(); ++i)
	{
		Renderable* renderable = m_temporaryRendeables[i];
		const Mesh* mesh = renderable->GetMesh();

		for (int subMeshIndex = 0; subMeshIndex < (int)mesh->GetSubMeshCount(); ++subMeshIndex)
		{
			DrawCall drawCall;
			drawCall.m_model			= renderable->GetModelMatrix();
			drawCall.m_mesh				= mesh->GetSubMesh(subMeshIndex);
			drawCall.m_material			= renderable->GetMaterial(subMeshIndex);
			drawCall.m_layer			= drawCall.m_material->GetShader()->GetRenderLayer();
			drawCall.m_queue			= drawCall.m_material->GetShader()->GetRenderQueue();
			drawCall.m_receivesShadows	= renderable->ReceivesShadows();

			// If is a lit material find the lights affecting it
			if (renderable->GetMaterial()->GetShader()->IsLit()) 
			{
				ComputeMostContributingLights( drawCall, scene ); 
			}

			drawCalls.push_back(drawCall); 
		}
	}
	Profiler_Pop();


	// Sort draw calls by layer/queue
	// sort alpha by distance to camera - extra
	SortDrawCalls(drawCalls, camera->GetForward());


	float cachedIntensity = 0.0f;
	if (!camera->DrawSun() && scene->GetMutableSun() != nullptr)
	{
		cachedIntensity = scene->GetMutableSun()->GetIntensity();
		scene->GetMutableSun()->Disable();
	}

	// Issue draw calls
	Profiler_Push("ForwardRenderPath::RenderForCamera.IssueDrawCalls");
	for (int i = 0; i < (int)drawCalls.size(); ++i)
	{
		DrawCall& drawCall = drawCalls[i];

		// Enable the correct lights for this draw call
		g_theRenderer->DisableAllPunctualLights();

		// Master Sun Light
		if (scene->GetMutableSun() != nullptr)
		{
			g_theRenderer->SetSunLight(scene->GetMutableSun(), drawCall.m_receivesShadows);
		}

		// Punctual Lights
		for (int li = 0; li < (int)drawCall.m_lightCount; ++li)
		{
			g_theRenderer->SetPunctualLight(li, drawCall.m_lights[li], drawCall.m_receivesShadows);
		}
		

		g_theRenderer->BindMaterial(drawCall.m_material); 
		g_theRenderer->BindModelMatrix(drawCall.m_model); 
		g_theRenderer->DrawMesh(drawCall.m_mesh); 
	}
	Profiler_Pop();


	// Issue debug draw calls
	if (camera->GetDrawDebugFlag() == true)
	{
		DebugDraw_Render3DForCamera(camera);
	}

	if (!camera->DrawSun()  && scene->GetMutableSun() != nullptr)
	{
		scene->GetMutableSun()->SetIntensity(cachedIntensity);
	}


	// Apply any screen effects
	TODO("Affects on cameras");
	//foreach (Material *effect in cam->m_effects) {
	//	m_renderer->ApplyEffect( effect ); 
	//}
	//m_renderer->FinishEffects(); 
}



void ForwardRenderPath::ClearForCamera(Camera* camera)
{
	PROFILE_SCOPE_FUNCTION();

	if (camera->ShouldClearDepth())
	{
		g_theRenderer->ClearDepth(camera->GetClearDepth());
	}

	switch(camera->GetColorClearMode())
	{
	case COLOR_CLEAR_MODE_COLOR:
	{
		g_theRenderer->ClearColor(camera->GetClearColor());
		break;
	}

	case COLOR_CLEAR_MODE_SKYBOX:
	{
		//g_theRenderer->ClearColor(RGBA());
		g_theRenderer->BindTextureCube(Renderer::SKYBOX_BIND_POINT, camera->GetClearSkybox());
		g_theRenderer->BindShader(m_skyboxShader);
		//g_theRenderer->BindModelMatrix(camera->GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(m_skyboxMesh->GetSubMesh(0));
		break;
	}

	default:
		break;
	}
}


static Vector3 comparator_cameraForward;
bool CompareDrawCalls(const DrawCall& lhs, const DrawCall& rhs)
{
	bool lhsIsFirst = false;

	if (lhs.m_layer < rhs.m_layer)
	{
		lhsIsFirst = true;
	}
	else if (lhs.m_queue < rhs.m_queue)
	{
		lhsIsFirst = true;
	}
	else if (lhs.m_queue == RENDER_QUEUE_ALPHA && rhs.m_queue == RENDER_QUEUE_ALPHA)
	{
		float lhsDistanceAlongCameraForward = DotProduct(comparator_cameraForward, lhs.m_model.GetTranslation());
		float rhsDistanceAlongCameraForward = DotProduct(comparator_cameraForward, rhs.m_model.GetTranslation());;
		if (lhsDistanceAlongCameraForward > rhsDistanceAlongCameraForward)
		{
			lhsIsFirst = true;
		}
	}

	return lhsIsFirst;
}



void ForwardRenderPath::SortDrawCalls(std::vector<DrawCall>& drawCalls, const Vector3& cameraForward)
{
	PROFILE_SCOPE_FUNCTION();

	comparator_cameraForward = cameraForward;
	std::sort(drawCalls.begin(), drawCalls.end(), CompareDrawCalls);
}


Vector3 renderableWorldPosition;
bool CompareLightContribution(const Light* lhs, const Light* rhs)
{
	bool lhsContributesMore = false;

	float lhsIntensity = lhs->GetIntensityAtPoint(renderableWorldPosition);
	float rhsIntensity = rhs->GetIntensityAtPoint(renderableWorldPosition);
	if (lhsIntensity > rhsIntensity)
	{
		lhsContributesMore = true;
	}

	return lhsContributesMore;
}



void ForwardRenderPath::ComputeMostContributingLights(DrawCall& drawCall, RenderScene* scene)
{
	PROFILE_SCOPE_FUNCTION();

	//renderableWorldPosition = renderable->GetModelMatrix().GetTranslation();
	//renderableWorldPosition = renderable->GetModelMatrix() * Vector4(drawCall->m_mesh)
	renderableWorldPosition = (drawCall.m_model * Vector4(drawCall.m_mesh->GetBounds().GetCenter(), 1.0f)).XYZ();

	std::vector<Light*>& lights = scene->GetMutableLights();
	std::sort(lights.begin(), lights.end(), CompareLightContribution);

	drawCall.m_lightCount = Min(MAX_LIGHTS, (int)lights.size());
	for (int i = 0; i < (int)drawCall.m_lightCount; ++i)
	{
		drawCall.m_lights[i] = lights[i];
	}
}



void ForwardRenderPath::AddTemporaryRenderable(Renderable* renderable)
{
	m_temporaryRendeables.push_back(renderable);
}



void ForwardRenderPath::ClearAllTemporaryRenderables()
{
	for (int i = 0; i < (int)m_temporaryRendeables.size(); ++i)
	{
		delete m_temporaryRendeables[i];
	}
	m_temporaryRendeables.clear();
}
