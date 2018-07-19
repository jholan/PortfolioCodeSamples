#pragma once

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/AABB2D.hpp"

class Camera;
class Texture;
class Material;



enum DebugRenderMode 
{
	DEBUG_RENDER_MODE_INVALID = -1,

	DEBUG_RENDER_IGNORE_DEPTH = 0,  // will always draw and be visible 
	DEBUG_RENDER_USE_DEPTH,			// draw using normal depth rules
	DEBUG_RENDER_HIDDEN,			// only draws if it would be hidden by depth
	DEBUG_RENDER_XRAY,				// always draws, but hidden area will be drawn differently

	DEBUG_RENDER_MODE_COUNT
};
Material* GetMaterialForDebugRenderMode(DebugRenderMode debugRenderMode);



struct DebugRenderOptions
{
	DebugRenderOptions()
		: m_startColor( RGBA() )
		, m_endColor( RGBA() )
		, m_lifespan(0.0f)
		, m_mode(DEBUG_RENDER_USE_DEPTH)
	{}

	RGBA			m_startColor; 
	RGBA			m_endColor; 
	float			m_lifespan; 
	DebugRenderMode m_mode; 
}; 



// Composition
void DebugDraw_Initialize();
void DebugDraw_Destroy();

// Core
void DebugDraw_BeginFrame();
void DebugDraw_EndFrame();
void DebugDraw_Update();
void DebugDraw_Render2D();
void DebugDraw_Render3DForCamera(Camera* camera);
void DebugDraw_RenderLog();
void DebugDraw_RemoveAllCompletedTasks();

void DebugDraw_SetVisibility(bool visibility);
void DebugDraw_ClearCommands();

// Grid
void DebugDraw_SetGridVisibility(bool visibility);
void DebugDraw_DrawGrid(unsigned int numGridLines = 10, unsigned int majorLineEvery = 10, const RGBA& majorLineColor = RGBA(150, 150, 150), const RGBA& minorLineColor = RGBA(50, 50, 50));

// Logging
void DebugDraw_Log( float lifespan, char const *format, ... );
void DebugDraw_Log( float lifespan, const RGBA& color, char const *format, ... );
void DebugDraw_Log( float lifespan, const RGBA& startColor, const RGBA& endColor, char const *format, ... );

// 2D Debug rendering options
void DebugDraw_2DQuad( float lifespan, const AABB2D& boundsPixels, const Texture* texture = nullptr, const RGBA& startColor = RGBA(), const RGBA& endColor = RGBA() ); 
void DebugDraw_2DLine( float lifespan, const Vector2& point0, const RGBA& point0Color, const Vector2& point1, const RGBA& point1Color, const RGBA& startTint, const RGBA& endTint );
void DebugDraw_2DText( float lifespan, const Vector2& position, float fontSize, const RGBA& startColor, const RGBA& endColor, char const *format, ... ); 

// 3D Debug rendering options
void DebugDraw_Point( float lifespan, const Vector3& position, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode ); 
void DebugDraw_LineSegment( float lifespan, const Vector3& point0, const RGBA& point0Color, const Vector3& point1, const RGBA& point1Color, const RGBA& startTint, const RGBA& endTint, DebugRenderMode const mode );
void DebugDraw_Basis( float lifespan, const Matrix4& basis, const RGBA& startTint, const RGBA& endTint, DebugRenderMode const mode ); 
void DebugDraw_WireSphere( float lifespan, const Vector3& position, float radius, const RGBA& startColor, const RGBA& endColor, DebugRenderMode const mode ); 
void DebugDraw_WireAABB3( float lifespan, const Vector3& position, const Vector3& halfDimensions, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode ); 
void DebugDraw_TexturedQuad( float lifespan, const Vector3& position, const Vector3& right, const Vector3& up, const Vector2& halfDimensions, const Texture* texture, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode ); 
void DebugDraw_3DText(float lifespan, const Vector3& position, const Vector2& pivot, const Vector3& right, const Vector3& up, float fontSize, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode, char const *format, ...);
void DebugDraw_3DTextBillboarded(float lifespan, const Vector3& position, const Vector2& pivot, float fontSize, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode, char const *format, ...);
void DebugDraw_Frustum(float lifespan, const Matrix4& projection, const Matrix4& cameraBasis, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode);


// "2.5D" Kinda weird needs both cameras
//		3D camera to transform position to screen space 
//		2D camera to draw rendering
//	Fail if no 3D camera is bound
void DebugDraw_TextTag( float lifespan, const Vector3& position, float fontSize, const RGBA& startColor, const RGBA& endColor, char const *format, ...);
void DebugDraw_Glyph( float lifespan, const Vector3& position, const AABB2D& dimensions, const Texture* texture, const RGBA& startTint, const RGBA& endTint);