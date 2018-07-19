#pragma once

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Rendering/FrameBuffer.hpp"
#include "Engine/Rendering/UniformBuffer.hpp"


class Texture;
class TextureCube;

enum ColorClearMode
{
	COLOR_CLEAR_MODE_INVALID = -1,

	COLOR_CLEAR_MODE_COLOR = 0,
	COLOR_CLEAR_MODE_SKYBOX,

	COLOR_CLEAR_MODE_COUNT
};



class Camera
{
public:
	Camera();
	~Camera();

	// Render Targets
	void				SetColorTarget( Texture* colorTarget );
	void				SetDepthStencilTarget( Texture* depthTarget );
	void				Finalize();

	// Camera Uniform Buffer
	unsigned int		GetFrameBufferHandle() const;
	void				UpdateUniformBuffer( unsigned int bindPoint );

	// Projection settings
	Matrix4				GetProjectionMatrix() const;
	void				SetProjection( Matrix4 projectionMatrix ); 
	void				SetProjectionOrtho( float verticalHalfSize, float nearClip, float farClip ); 
	void				SetProjectionPerspective( float fov_degrees, float aspect, float nz, float fz );

	// Shadow Cascades
	void				SetCascadeBreakpoints(float* cascadeBreakpoints); // Will attempt to read NUM_CASCADES floats from the pointer
	std::vector<float>  GetCascadeBreakpoints() const;
	const Matrix4&		GetProjectionForCascade(int i) const;

	// Transform
	void				LookAt( Vector3 cameraPosition, Vector3 targetPosition, Vector3 up = Vector3(0.0f, 1.0f, 0.0f) ); 
	void				SetLocalToParentMatrix( const Matrix4& transform );
	Matrix4				GetWorldToLocalMatrix() const;
	Matrix4				GetLocalToWorldMatrix() const;
	Vector3				GetForward()  const;
	Vector3				GetRight()	  const;
	Vector3				GetUp()		  const;
	Vector3				GetPosition() const;

	// Color Clearing
	void				SetColorClearMode(ColorClearMode mode);
	ColorClearMode		GetColorClearMode() const;

	void				SetClearColor(const RGBA& color);
	RGBA				GetClearColor() const;
	
	void				SetClearSkybox(const TextureCube* skybox);
	const TextureCube*	GetClearSkybox() const;

	// Depth Clearing
	void				SetDepthClearOptions( bool clearsDepth, float depth = 1.0f );
	bool				ShouldClearDepth();
	float				GetClearDepth();
	
	// Screen Picking
	IntVector2			WorldToScreen(const Vector3& worldPosition) const;
	Vector3				ScreenToWorld(const IntVector2& screenPosition, float depth, bool isNDCDepth = false) const;

	// Viewport
	void				SetCameraOrder(unsigned int order);
	unsigned int		GetCameraOrder() const;

	void				SetViewportNormalizedSize(const AABB2D& viewportSize); // DBAD this doesn't check for normalized values
	IntVector2			GetViewportDimensions() const;
	IntVector2			GetViewportLowerLeft() const;
	IntVector2			GetViewportUpperRight() const;

	void				OverrideViewportBaseResolution(bool shouldOverride, IntVector2 resolution); // if true resolution will be used instead of screen dimensions 

	Camera*				CreateScreenSpaceCameraForViewport() const;

	// Debug Drawing
	bool				GetDrawDebugFlag() const;
	void				SetDebugDrawFlag(bool shouldDraw);

	bool				DrawSun() const;
	void				ShouldDrawSun(bool shouldUpdate);


public:
	// default all to identity
	Matrix4				m_localToWorld;			 // where is the camera?
	Matrix4				m_worldToLocal;			 // inverse of camera (used for shader)
	Matrix4				m_projection;			 // projection

	float				m_nearPlane;
	float				m_farPlane;

	std::vector<float>  m_cascadeBreakpoints;
	Matrix4				m_cascadeProjections[NUM_CASCADES];

	bool				m_clearDepth			 = false;
	float				m_depthToClearTo		 = 1.0f;

	ColorClearMode		m_clearMode				 = COLOR_CLEAR_MODE_COLOR;
	RGBA				m_clearColor			 = RGBA(0,0,0);
	const TextureCube*	m_skyboxTex				 = nullptr;

	FrameBuffer			m_output;	
	UniformBlock		m_cameraBuffer;

	unsigned int		m_drawOrder				 = 0;
	bool				m_debugDrawFlag			 = true;
	AABB2D				m_normalizedViewportSize = AABB2D::ZERO_TO_ONE;

	bool				m_shouldOverrideBaseResolution = false;
	IntVector2			m_overrideResolution;

	bool				m_updateSun				 = true;
}; 