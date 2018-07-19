#include "Engine/Rendering/Camera.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector4.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Core/Window.hpp"
extern Window* g_theWindow;



struct CameraUniformBlock
{
	Matrix4 view;
	Matrix4 projection;

	Vector3 eyePosition;
	float	padding_0;
};



Camera::Camera() 
{
	CameraUniformBlock cameraUniformBlock;
	cameraUniformBlock.projection = m_projection;
	cameraUniformBlock.view = m_worldToLocal;
	cameraUniformBlock.eyePosition = GetPosition();
	
	m_cameraBuffer.Set<CameraUniformBlock>(cameraUniformBlock);
}



Camera::~Camera()
{

}



void Camera::SetColorClearMode(ColorClearMode mode)
{
	// If they give us count just dont clear color
	if ( mode == COLOR_CLEAR_MODE_COUNT)
	{
		mode = COLOR_CLEAR_MODE_INVALID;
	}

	m_clearMode = mode;
}



ColorClearMode Camera::GetColorClearMode() const
{
	return m_clearMode;
}



void Camera::SetClearColor(const RGBA& color)
{
	m_clearColor = color;
}



RGBA Camera::GetClearColor() const
{
	return m_clearColor;
}



void Camera::SetClearSkybox(const TextureCube* skybox)
{
	m_skyboxTex = skybox;
}



const TextureCube* Camera::GetClearSkybox() const
{
	return m_skyboxTex;
}



void Camera::SetDepthClearOptions( bool clearsDepth, float depth )
{
	m_clearDepth = clearsDepth;
	m_depthToClearTo = depth;
}



bool Camera::ShouldClearDepth()
{
	return m_clearDepth;
}



float Camera::GetClearDepth()
{
	return m_depthToClearTo;
}



IntVector2 Camera::WorldToScreen(const Vector3& worldPosition) const
{
	Vector4 ndcPosition = GetProjectionMatrix() * GetWorldToLocalMatrix() * Vector4(worldPosition, 1.0f);
	ndcPosition /= ndcPosition.z;

	//IntVector2 windowDimensions = g_theWindow->GetDimensions();
	IntVector2 windowDimensions = GetViewportDimensions();
	float screenX = RangeMapFloat(ndcPosition.x, -1.0f, 1.0f, 0.0f, (float)windowDimensions.x);
	float screenY = RangeMapFloat(ndcPosition.y, -1.0f, 1.0f, 0.0f, (float)windowDimensions.y);

	IntVector2 screenPosition = IntVector2((int)screenX, (int)screenY);
	return screenPosition;
}



Vector3 Camera::ScreenToWorld(const IntVector2& screenPosition, float depth, bool isNDCDepth) const
{
	float ndcDepth = depth;
	if (!isNDCDepth)
	{
		Vector4 ndcPosition = GetProjectionMatrix() * GetWorldToLocalMatrix() * Vector4(0.0f, 0.0f, depth, 1.0f);
		ndcPosition /= ndcPosition.z;
	}

	IntVector2 screenDim = GetViewportDimensions();
	float ndcX = RangeMapFloat( (float)screenPosition.x, 0.0f, (float)screenDim.x, -1.0f, 1.0f);
	float ndcY = RangeMapFloat( (float)screenPosition.y, 0.0f, (float)screenDim.y, -1.0f, 1.0f);

	Vector3 ndc = Vector3(ndcX, ndcY, ndcDepth); 

	Matrix4 inverseViewProjectionMatrix = GetProjectionMatrix() * GetWorldToLocalMatrix();
	inverseViewProjectionMatrix.Invert();
	Vector4 homogeneousWorldPosition = inverseViewProjectionMatrix * Vector4( ndc, 1.0f ); 
	Vector3 worldPosition = Vector3(homogeneousWorldPosition.x, homogeneousWorldPosition.y, homogeneousWorldPosition.z) / homogeneousWorldPosition.w;

	return worldPosition;
}



void Camera::SetCameraOrder(unsigned int order)
{
	m_drawOrder = order;
}



unsigned int Camera::GetCameraOrder() const
{
	return m_drawOrder;
}



void Camera::SetViewportNormalizedSize(const AABB2D& viewportSize)
{
	m_normalizedViewportSize = viewportSize;
}



IntVector2 Camera::GetViewportDimensions() const
{
	// Get the viewport resolution
	IntVector2 screenDimensions = g_theWindow->GetDimensions();
	if (m_shouldOverrideBaseResolution)
	{
		screenDimensions = m_overrideResolution;
	}

	// Range map the viewports dimensions
	float xRange = m_normalizedViewportSize.maxs.x - m_normalizedViewportSize.mins.x;
	int xViewportDim = (int)((float)screenDimensions.x * xRange);
	
	float yRange = m_normalizedViewportSize.maxs.y - m_normalizedViewportSize.mins.y;
	int yViewportDim = (int)((float)screenDimensions.y * yRange);

	IntVector2 viewportDimensions = IntVector2(xViewportDim, yViewportDim);
	return viewportDimensions;
}



IntVector2 Camera::GetViewportLowerLeft() const
{
	// Get the viewport resolution
	IntVector2 screenDimensions = g_theWindow->GetDimensions();
	if (m_shouldOverrideBaseResolution)
	{
		screenDimensions = m_overrideResolution;
	}

	// Range map the viewports min values
	int xMin = (int)RangeMapFloat(m_normalizedViewportSize.mins.x, 0.0f, 1.0f, 0.0f, (float)screenDimensions.x);
	int yMin = (int)RangeMapFloat(m_normalizedViewportSize.mins.y, 0.0f, 1.0f, 0.0f, (float)screenDimensions.y);

	IntVector2 viewportLowerLeft = IntVector2(xMin, yMin);
	return viewportLowerLeft;
}



IntVector2 Camera::GetViewportUpperRight() const
{
	// Get the viewport resolution
	IntVector2 screenDimensions = g_theWindow->GetDimensions();
	if (m_shouldOverrideBaseResolution)
	{
		screenDimensions = m_overrideResolution;
	}

	// Range map the viewports max values
	int xMax = (int)RangeMapFloat(m_normalizedViewportSize.maxs.x, 0.0f, 1.0f, 0.0f, (float)screenDimensions.x);
	int yMax = (int)RangeMapFloat(m_normalizedViewportSize.maxs.y, 0.0f, 1.0f, 0.0f, (float)screenDimensions.y);

	IntVector2 viewportUpperRight = IntVector2(xMax, yMax);
	return viewportUpperRight;
}



void Camera::OverrideViewportBaseResolution(bool shouldOverride, IntVector2 resolution)
{
	m_shouldOverrideBaseResolution = shouldOverride;
	m_overrideResolution = resolution;
}



Camera*	Camera::CreateScreenSpaceCameraForViewport() const
{
	Camera* screenSpaceCamera = new Camera();
	screenSpaceCamera->SetColorTarget(m_output.m_colorTarget);
	screenSpaceCamera->SetDepthStencilTarget(m_output.m_depthStencilTarget);
	screenSpaceCamera->SetProjection(Matrix4::MakeOrtho2D(Vector2(0.0f, 0.0f), (Vector2)GetViewportDimensions()));

	screenSpaceCamera->m_clearDepth = m_clearDepth;
	screenSpaceCamera->m_depthToClearTo = m_depthToClearTo;

	screenSpaceCamera->m_clearMode = m_clearMode;
	screenSpaceCamera->m_clearColor = m_clearColor;
	screenSpaceCamera->m_skyboxTex = m_skyboxTex;

	screenSpaceCamera->m_normalizedViewportSize = m_normalizedViewportSize;
	
	return screenSpaceCamera;
}



bool Camera::GetDrawDebugFlag() const
{
	return m_debugDrawFlag;
}



void Camera::SetDebugDrawFlag(bool shouldDraw)
{
	m_debugDrawFlag = shouldDraw;
}



bool Camera::DrawSun() const
{
	return m_updateSun;
}



void Camera::ShouldDrawSun(bool shouldUpdate)
{
	m_updateSun = shouldUpdate;
}



void Camera::SetColorTarget( Texture* colorTarget )
{
	m_output.SetColorTarget(colorTarget);
}



void Camera::SetDepthStencilTarget( Texture* depthTarget )
{
	m_output.SetDepthStencilTarget(depthTarget);
}



void Camera::Finalize()
{
	m_output.Finalize();
}



Vector3 Camera::GetForward() const
{
	return m_localToWorld.GetForward();
}



Vector3 Camera::GetRight() const
{
	return m_localToWorld.GetRight();
}



Vector3 Camera::GetUp() const
{
	return m_localToWorld.GetUp();
}



Vector3 Camera::GetPosition() const
{
	return m_localToWorld.GetTranslation();
}



unsigned int Camera::GetFrameBufferHandle() const
{
	return m_output.GetHandle();
}



void Camera::UpdateUniformBuffer(unsigned int bindPoint)
{
	m_cameraBuffer.BindTo(bindPoint);
}



void Camera::LookAt( Vector3 cameraPosition, Vector3 targetPosition, Vector3 up)
{
	m_localToWorld = Matrix4::CreateLookAt(cameraPosition, targetPosition, up);
	m_worldToLocal = Matrix4::CreateLookAtInverse(cameraPosition, targetPosition, up);

	// Update UniformBuffer
	CameraUniformBlock* cub = m_cameraBuffer.As<CameraUniformBlock>();
	cub->view = m_worldToLocal;
	cub->eyePosition = GetPosition();
}



Matrix4 Camera::GetProjectionMatrix() const
{
	return m_projection;
}



void Camera::SetProjection( Matrix4 projectionMatrix )
{
	m_projection = projectionMatrix;

	// Update UniformBuffer
	CameraUniformBlock* cub = m_cameraBuffer.As<CameraUniformBlock>();
	cub->projection = m_projection;
	cub->eyePosition = GetPosition();
}



void Camera::SetProjectionOrtho( float verticalHalfSize, float nearClip, float farClip )
{
	// Calculate the aspect ratio based on the frame buffer size	
	IntVector2 frameBufferDimensions = m_output.GetDimensions();
	GUARANTEE_OR_DIE(frameBufferDimensions.y != 0, "Framebuffer dimensions.y = 0");

	float frameBufferAspect = (float)frameBufferDimensions.x / (float)frameBufferDimensions.y;


	// Save off near far planes
	m_nearPlane = nearClip;
	m_farPlane = farClip;


	// The primary view projection
	m_projection = Matrix4::CreateOrthographicProjection(-verticalHalfSize * frameBufferAspect, verticalHalfSize * frameBufferAspect, -verticalHalfSize, verticalHalfSize, nearClip, farClip);
	

	// Set up cascades
	if (m_cascadeBreakpoints.size() == 0)
	{
		// If no cascades are set up initially do a crappy approximation
		int	  numCascades = NUM_CASCADES;
		float range = m_farPlane - m_nearPlane;
		float rangePerCascade = range / (float)numCascades;

		float* cascadeBreakpoints = new float[numCascades];
		float cascadeDepth = 0.0f;
		for (int i = 0; i < numCascades; ++i)
		{
			cascadeDepth += rangePerCascade;
			cascadeBreakpoints[i] = cascadeDepth;
		}
		SetCascadeBreakpoints(cascadeBreakpoints);
	}

	// Create the cascade projections
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		float cascadeNear = m_cascadeBreakpoints[i];
		float cascadeFar = m_cascadeBreakpoints[i + 1];

		m_cascadeProjections[i] = Matrix4::CreateOrthographicProjection(-verticalHalfSize * frameBufferAspect, verticalHalfSize * frameBufferAspect, -verticalHalfSize, verticalHalfSize, cascadeNear, cascadeFar);
	}


	// Update UniformBuffer
	CameraUniformBlock* cub = m_cameraBuffer.As<CameraUniformBlock>();
	cub->projection = m_projection;
	cub->eyePosition = GetPosition();
}



void Camera::SetProjectionPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane)
{
	TODO("Remove aspect as we are calculating it ourselves");
	UNUSED(aspect);
	
	// Calculate the aspect ratio based on the frame buffer size	
	IntVector2 frameBufferDimensions = m_output.GetDimensions();
	GUARANTEE_OR_DIE(frameBufferDimensions.y != 0, "Framebuffer dimensions.y = 0");

	float frameBufferAspect = (float)frameBufferDimensions.x / (float)frameBufferDimensions.y;


	// Save off near far planes
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;


	// The primary view projection
	m_projection = Matrix4::CreatePerspectiveProjection(fovDegrees, frameBufferAspect, nearPlane, farPlane);


	// Set up cascades
	if (m_cascadeBreakpoints.size() == 0)
	{
		// If no cascades are set up initially do a crappy approximation
		int	  numCascades = NUM_CASCADES;
		float range = farPlane - nearPlane;
		float rangePerCascade = range / (float)numCascades;

		float* cascadeBreakpoints = new float[numCascades];
		float cascadeDepth = 0.0f;
		for (int i = 0; i < numCascades; ++i)
		{
			cascadeDepth += rangePerCascade;
			cascadeBreakpoints[i] = cascadeDepth;
		}
		SetCascadeBreakpoints(cascadeBreakpoints);
	}
	m_cascadeBreakpoints[0] = nearPlane;

	// Create the cascade projections
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		float cascadeNear = m_cascadeBreakpoints[i];
		float cascadeFar = m_cascadeBreakpoints[i + 1];

		m_cascadeProjections[i] = Matrix4::CreatePerspectiveProjection(fovDegrees, frameBufferAspect, cascadeNear, cascadeFar);
	}


	// Update UniformBuffer
	CameraUniformBlock* cub = m_cameraBuffer.As<CameraUniformBlock>();
	cub->projection = m_projection;
	cub->eyePosition = GetPosition();
}



void Camera::SetCascadeBreakpoints(float* cascadeBreakpoints)
{
	m_cascadeBreakpoints.clear();


	// Add the near plane by default
	m_cascadeBreakpoints.push_back(m_nearPlane);


	// Add the requested depths
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		m_cascadeBreakpoints.push_back(cascadeBreakpoints[i]);
	}
}



std::vector<float> Camera::GetCascadeBreakpoints() const
{
	std::vector<float> depths;

	// Skip the near plane
	for (int i = 1; i < (int)m_cascadeBreakpoints.size(); ++i)
	{
		depths.push_back(m_cascadeBreakpoints[i]);
	}

	return depths;
}



const Matrix4& Camera::GetProjectionForCascade(int i) const
{
	GUARANTEE_OR_DIE(i >= 0 && i < NUM_CASCADES, "Invalid cascade number");

	return m_cascadeProjections[i];
}



void Camera::SetLocalToParentMatrix(const Matrix4& transform)
{
	m_localToWorld = transform;
	
	m_worldToLocal = m_localToWorld;
	m_worldToLocal.Invert();

	// Update UniformBuffer
	CameraUniformBlock* cub = m_cameraBuffer.As<CameraUniformBlock>();
	cub->view = m_worldToLocal;
	cub->eyePosition = GetPosition();
}



Matrix4 Camera::GetWorldToLocalMatrix() const
{
	return m_worldToLocal;
}



Matrix4 Camera::GetLocalToWorldMatrix() const
{
	return m_localToWorld;
}
