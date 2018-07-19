#include "Engine/Rendering/DebugRender.hpp"

#include <stdarg.h>
#include <vector>
#include <string>

#include "Engine/Profiler/ProfileScope.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/MeshBuilder.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Commands/Command.hpp"
#include "Engine/Commands/DevConsole.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Transform.hpp"


extern Clock*		g_theMasterClock;
extern Renderer*	g_theRenderer;
extern DevConsole*	g_theDevConsole;
extern Window*		g_theWindow;

class	DebugRenderTask;
void	RemoveAllTasks(std::vector<DebugRenderTask*>& tasks);



// State
static std::vector<DebugRenderTask*> s_debugRenderTasks_2D;
static std::vector<DebugRenderTask*> s_debugRenderTasks_3D;
static std::vector<DebugRenderTask*> s_debugRenderTasks_3D_ToScreen;
static bool							 s_render = true;
static Camera*						 s_camera3D = nullptr;
static Camera*						 s_camera2D = nullptr;
static const BitmapFont*			 s_font = nullptr;
static Shader*						 s_debugShader_2D = nullptr;
static Shader*						 s_debugShader_3D_depth = nullptr;
static Shader*						 s_debugShader_3D_noDepth = nullptr;
static const Texture*				 s_defaultTexture = nullptr;

static bool							 s_drawGrid			= false;
static unsigned int					 s_numGridLines		= 10;
static unsigned int					 s_majorLineEvery	= 10;
static float						 s_gridLineHalfSize = 5.0f;
static RGBA							 s_majorLineColor	= RGBA(50, 50, 50);
static RGBA							 s_minorLineColor	= RGBA(150, 150, 150);

static Mesh*						 s_pointMesh = nullptr;
static Mesh*						 s_basisMesh = nullptr;
static Mesh*						 s_sphereMesh = nullptr;
static Mesh*						 s_cubeMesh = nullptr;

static std::vector<DebugRenderTask*> s_log;
static Vector2						 s_logAncor;
static float						 s_logFontSize = 18.0f;



// Console Commands
void DebugRenderClearRenderCommands_Command(Command& cmd)
{
	UNUSED(cmd);

	DebugDraw_ClearCommands();
}



void DebugRenderSetVisibility_Command(Command& cmd)
{
	bool visibility = StringToBool(cmd.GetNextString().c_str());

	DebugDraw_SetVisibility(visibility);
}



void DebugRenderToggleGrid_Command(Command& cmd)
{
	UNUSED(cmd);

	bool visibility = !s_drawGrid;
	DebugDraw_SetGridVisibility(visibility);
}



Material* GetMaterialForDebugRenderMode(DebugRenderMode debugRenderMode)
{
	Material* material = nullptr;

	switch(debugRenderMode)
	{
	case DEBUG_RENDER_IGNORE_DEPTH:
	{
		material = Material::Get("DebugDraw_3D_noDepth");
		break;
	}

	case DEBUG_RENDER_USE_DEPTH:
	{
		material = Material::Get("DebugDraw_3D_depth");
		break;
	}

	case DEBUG_RENDER_HIDDEN:
	{
		GUARANTEE_OR_DIE(false, "HIDDEN DebugRenderMode specified, Mode not currently supported");
		break;
	}

	case DEBUG_RENDER_XRAY:
	{
		GUARANTEE_OR_DIE(false, "XRAY DebugRenderMode specified, Mode not currently supported");
		break;
	}

	default:
	{
		GUARANTEE_OR_DIE(false, "Invalid DebugRenderMode specified");
		break;
	}
	}

	return material;
}



class DebugRenderTask
{
public:
	virtual ~DebugRenderTask() {};

	virtual void Update(float deltaSeconds);
	virtual void Render() const = 0;

	bool CanBeRemoved() const;
	float GetPercentComplete() const;
	RGBA GetCurrentColor() const;

	float			   m_lifetime = 0.0f;
	DebugRenderOptions m_options;
};

void DebugRenderTask::Update(float deltaSeconds)
{
	m_lifetime += deltaSeconds;
}

bool DebugRenderTask::CanBeRemoved() const
{
	bool canBeRemoved = false;

	if(m_lifetime > m_options.m_lifespan)
	{
		canBeRemoved = true;
	}

	return canBeRemoved;
}

float DebugRenderTask::GetPercentComplete() const
{
	float percentComplete = 1.0f;

	if (m_options.m_lifespan != 0.0f)
	{
		percentComplete = m_lifetime / m_options.m_lifespan;
	}

	return percentComplete;
}

RGBA DebugRenderTask::GetCurrentColor() const
{
	RGBA color = Interpolate(m_options.m_startColor, m_options.m_endColor, GetPercentComplete());
	return color;
}

class DebugRenderTask_Quad : public DebugRenderTask
{
public:
	DebugRenderTask_Quad(float lifespan, const AABB2D& bounds, const Texture* texture, const RGBA& startColor, const RGBA& endColor) 
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;

		m_mesh = CreateAABB2D(bounds, RGBA());
		m_material = Material::Get("DebugDraw_2D");

		if (texture != nullptr)
		{
			m_material->SetDiffuseTexture(texture);
		}
	};

	~DebugRenderTask_Quad()
	{
		delete m_mesh;
		m_mesh = nullptr;

		delete m_material;
		m_material = nullptr;
	}

	void Render() const 
	{
		m_material->SetProperty("TINT", GetCurrentColor());

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(Matrix4());
		g_theRenderer->DrawMesh(m_mesh->GetSubMesh(0));
	};

	Mesh*		m_mesh		= nullptr;
	Material*	m_material	= nullptr;
};

class DebugRenderTask_Line : public DebugRenderTask
{
public:
	DebugRenderTask_Line(float lifespan, const Vector2& point0, const RGBA& point0Color, const Vector2& point1, const RGBA& point1Color, const RGBA& startTint, const RGBA& endTint ) 
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startTint;
		m_options.m_endColor	= endTint;

		m_material = Material::Get("DebugDraw_2D");
		m_mesh = CreateLine(point0, point0Color, point1, point1Color);
	};

	~DebugRenderTask_Line()
	{
		delete m_mesh;
		m_mesh = nullptr;

		delete m_material;
		m_material = nullptr;
	}

	void Render() const 
	{
		m_material->SetProperty("TINT", GetCurrentColor());

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(Matrix4());
		g_theRenderer->DrawMesh(m_mesh->GetSubMesh(0));
	}

	Mesh*		m_mesh		= nullptr;
	Material*	m_material	= nullptr;
};

class DebugRenderTask_Text : public DebugRenderTask
{
public:
	DebugRenderTask_Text(float lifespan, const Vector2& lowerLeft, float fontSize, const RGBA& startColor, const RGBA& endColor, const std::string& text )
		: m_lowerLeft(lowerLeft)
		, m_fontSize(fontSize)
		, m_text(text)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;

		m_material = Material::Get("DebugDraw_2D");
	};

	~DebugRenderTask_Text()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const 
	{
		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(Matrix4());

		float aspect = 1.0f;
		RGBA color = GetCurrentColor();
		g_theRenderer->DrawText2D(m_lowerLeft, m_text, m_fontSize, color, aspect, s_font);
	}

	Vector2		m_lowerLeft;
	float		m_fontSize;
	std::string m_text;

	Material* m_material = nullptr;
};

class DebugRenderTask_Point : public DebugRenderTask
{
public:
	DebugRenderTask_Point(float lifespan, const Vector3& position, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;
		m_options.m_mode		= mode;

		m_transform.SetLocalPosition(position);
		m_material = GetMaterialForDebugRenderMode(mode);
	};

	~DebugRenderTask_Point()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const
	{
		// Update
		m_material->SetProperty("TINT", GetCurrentColor());

		// Draw 
		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(m_transform.GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(s_pointMesh->GetSubMesh(0));
	};

	Transform	m_transform;
	Material*	m_material		= nullptr;
};

class DebugRenderTask_LineSegment : public DebugRenderTask
{
public:
	DebugRenderTask_LineSegment(float lifespan, const Vector3& point0, const RGBA& point0Color, const Vector3& point1, const RGBA& point1Color, const RGBA& startTint, const RGBA& endTint, DebugRenderMode const mode)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startTint;
		m_options.m_endColor	= endTint;
		m_options.m_mode		= mode;

		m_transform = Transform();
		m_material = GetMaterialForDebugRenderMode(mode);
		
		// Mesh
		MeshBuilder builder = MeshBuilder();
		builder.Initialize(Renderer::PRIMITIVE_LINES, false);

		builder.SetVertexUVs(0.0f, 0.0f);

		builder.SetVertexColor(point0Color);
		builder.PushVertex(point0);

		builder.SetVertexColor(point1Color);
		builder.PushVertex(point1);

		builder.Finalize();
		m_mesh = builder.CreateMesh();
	};

	~DebugRenderTask_LineSegment()
	{
		delete m_material;
		m_material = nullptr;

		delete m_mesh;
		m_mesh = nullptr;
	}

	void Render() const
	{
		m_material->SetProperty("TINT", GetCurrentColor());

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(m_transform.GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(m_mesh->GetSubMesh(0));
	};

	Transform	m_transform;
	Mesh*		m_mesh = nullptr;
	Material*	m_material = nullptr;
};

class DebugRenderTask_Basis : public DebugRenderTask
{
public:
	DebugRenderTask_Basis(float lifespan, const Matrix4& basis, const RGBA& startTint, const RGBA& endTint, DebugRenderMode const mode)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startTint;
		m_options.m_endColor	= endTint;
		m_options.m_mode		= mode;

		m_transform.SetFromMatrix(basis);
		m_material = GetMaterialForDebugRenderMode(mode);
	};

	~DebugRenderTask_Basis()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const
	{
		m_material->SetProperty("TINT", GetCurrentColor());

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(m_transform.GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(s_basisMesh->GetSubMesh(0));
	};

	Transform m_transform;
	Material* m_material = nullptr;
};

class DebugRenderTask_WireSphere : public DebugRenderTask
{
public:
	DebugRenderTask_WireSphere(float lifespan, const Vector3& position, float radius, const RGBA& startColor, const RGBA& endColor, DebugRenderMode const mode)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;
		m_options.m_mode		= mode;

		m_transform = Transform();
		m_transform.SetWorldPosition(position);
		m_transform.SetLocalScale(radius, radius, radius);
		m_material = GetMaterialForDebugRenderMode(mode);
	}

	~DebugRenderTask_WireSphere()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const
	{
		m_material->SetProperty("TINT", GetCurrentColor());

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(m_transform.GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(s_sphereMesh->GetSubMesh(0));
	}

	Transform m_transform;
	Material* m_material = nullptr;
};

class DebugRenderTask_WireAABB3 : public DebugRenderTask
{
public:
	DebugRenderTask_WireAABB3(float lifespan, const Vector3& position, const Vector3& halfDimensions, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;
		m_options.m_mode		= mode;

		m_transform.SetWorldPosition(position);
		m_transform.SetLocalScale(halfDimensions);
		m_material = GetMaterialForDebugRenderMode(mode);
	}

	~DebugRenderTask_WireAABB3()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const
	{
		m_material->SetProperty("TINT", GetCurrentColor());

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(m_transform.GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(s_cubeMesh->GetSubMesh(0));
	}

	Transform m_transform;
	Material* m_material = nullptr;
};

class DebugRenderTask_TexturedQuad : public DebugRenderTask
{
public:
	DebugRenderTask_TexturedQuad(float lifespan, Vector3 const &position, Vector3 const &right, Vector3 const &up, const Vector2& halfDimensions, const Texture* texture, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;
		m_options.m_mode		= mode;

		m_material = Material::Get("DebugDraw_2D");
		m_material->SetDiffuseTexture(texture);

		// Build Mesh
		// Generate corners
		Vector3 ll = position - (right * halfDimensions.x) - (up * halfDimensions.y);
		Vector3 lr = position + (right * halfDimensions.x) - (up * halfDimensions.y);
		Vector3 ur = position + (right * halfDimensions.x) + (up * halfDimensions.y);
		Vector3 ul = position - (right * halfDimensions.x) + (up * halfDimensions.y);

		MeshBuilder builder = MeshBuilder();
		builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, true);

		builder.SetVertexColor(RGBA());

		builder.SetVertexUVs(0.0f, 0.0f);
		unsigned int index = builder.PushVertex(ll);

		builder.SetVertexUVs(1.0f, 0.0f);
		builder.PushVertex(lr);

		builder.SetVertexUVs(1.0f, 1.0f);
		builder.PushVertex(ur);

		builder.SetVertexUVs(0.0f, 1.0f);
		builder.PushVertex(ul);

		builder.AddQuad(index + 0, index + 1, index + 2, index + 3);

		builder.Finalize();
		m_mesh = builder.CreateMesh();
	}

	~DebugRenderTask_TexturedQuad()
	{
		delete m_mesh;
		m_mesh = nullptr;

		delete m_material;
		m_material = nullptr;
	}

	void Render() const
	{
		m_material->SetProperty("TINT", GetCurrentColor());

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(Matrix4());
		g_theRenderer->DrawMesh(m_mesh->GetSubMesh(0));
	}

	Material*	m_material	= nullptr;
	Mesh*		m_mesh		= nullptr;
};

class DebugRenderTask_3DText : public DebugRenderTask
{
public:
	DebugRenderTask_3DText(float lifespan, const Vector3& position, const Vector2& pivot, const Vector3& right, const Vector3& up, float fontSize, const RGBA& startColor, const RGBA& endColor, const std::string& text, DebugRenderMode mode, bool isBillboarded = false )
		: m_worldPosition(position)
		, m_right(right)
		, m_up(up)
		, m_fontSize(fontSize)
		, m_text(text)
		, m_billboardToCamera(isBillboarded)
		, m_pivot(pivot)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;
		m_options.m_mode		= mode;

		m_material = Material::Get("DebugDraw_2D");
	};

	~DebugRenderTask_3DText()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const 
	{
		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(Matrix4());

		Vector3 right = m_right;
		Vector3 up = m_up;
		if (m_billboardToCamera)
		{
			right = s_camera3D->GetRight();
			up = s_camera3D->GetUp();
		}

		TODO("Pivot in DrawText3D");
		float textLength = s_font->GetStringWidth(m_text, m_fontSize, m_aspect);
		Vector3 position = m_worldPosition + (-right * textLength * m_pivot.x) + (-up * m_fontSize * m_pivot.y);
		RGBA color = GetCurrentColor();
		g_theRenderer->DrawText3D(position, right, up, m_text, m_fontSize, color, m_aspect, s_font);
	}

	Vector3		m_worldPosition;
	Vector2		m_pivot;
	Vector3		m_right;
	Vector3		m_up;
	std::string m_text;
	float		m_fontSize;
	float		m_aspect = 1.0f;
	bool		m_billboardToCamera = false;

	Material* m_material = nullptr;
};

class DebugRenderTask_Grid : public DebugRenderTask
{
public:
	DebugRenderTask_Grid(float lifespan, const Vector3& halfDimensions, float lineLength = 10.0f, int majorLinesEvery = 10, const RGBA& majorColor = RGBA(50, 50, 50), const RGBA& minorColor = RGBA(150, 150, 150))
		: m_halfDimensions(halfDimensions)
		, m_majorLinesEvery(majorLinesEvery)
		, m_majorColor(majorColor)
		, m_minorColor(minorColor)
		, m_lineLength(lineLength)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_mode		= DEBUG_RENDER_USE_DEPTH;

		m_transform = Transform();
		m_material = GetMaterialForDebugRenderMode(m_options.m_mode);
	}

	~DebugRenderTask_Grid()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const
	{
		// Short Circuit
		if (s_camera3D == nullptr)
		{
			return;
		}

		Vector3 camPos = s_camera3D->GetPosition();
		IntRange xRange = IntRange((int)ceil(camPos.x - m_halfDimensions.x), (int)floor(camPos.x + m_halfDimensions.x));
		IntRange zRange = IntRange((int)ceil(camPos.z - m_halfDimensions.z), (int)floor(camPos.z + m_halfDimensions.z));


		MeshBuilder builder = MeshBuilder();
		builder.Initialize(Renderer::PRIMITIVE_LINES, false);

		for (int x = xRange.min; x <= xRange.max; ++x)
		{
			if(x == 0)
			{
				builder.SetVertexColor(RGBA(0,0,255));
			}
			else if (x % m_majorLinesEvery == 0)
			{
				builder.SetVertexColor(m_majorColor);
			}
			else
			{
				builder.SetVertexColor(m_minorColor);
			}

			Vector3 pos1 = Vector3((float)x, 0.0f, camPos.z - m_halfDimensions.z);
			Vector3 pos2 = Vector3((float)x, 0.0f, camPos.z + m_halfDimensions.z);
			builder.AddLine(pos1, pos2);
		}

		for (int z = zRange.min; z <= zRange.max; ++z)
		{
			if(z == 0)
			{
				builder.SetVertexColor(RGBA(255,0,0));
			}
			else if (z % m_majorLinesEvery == 0)
			{
				builder.SetVertexColor(m_majorColor);
			}
			else
			{
				builder.SetVertexColor(m_minorColor);
			}

			Vector3 pos1 = Vector3(camPos.x - m_halfDimensions.x, 0.0f, (float)z);
			Vector3 pos2 = Vector3(camPos.x + m_halfDimensions.x, 0.0f, (float)z);
			builder.AddLine(pos1, pos2);
		}


		builder.Finalize();
		Mesh* mesh = builder.CreateMesh();

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(m_transform.GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(mesh->GetSubMesh(0));
		delete mesh;
	}

	Transform m_transform;
	Material* m_material = nullptr;

	Vector3 m_halfDimensions;
	int		m_majorLinesEvery;
	float	m_lineLength;

	RGBA	m_majorColor;
	RGBA	m_minorColor;
};

class DebugRenderTask_Frustum : public DebugRenderTask
{
public:
	DebugRenderTask_Frustum(float lifespan, const Matrix4& projection, const Matrix4& cameraBasis, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode)
		: m_transform(cameraBasis)
		, m_projectionPoints(GetPointsForProjection(projection))
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;
		m_options.m_mode		= mode;

		//m_transform = Transform();
		m_material = GetMaterialForDebugRenderMode(m_options.m_mode);
	}

	~DebugRenderTask_Frustum()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const
	{
		// Short Circuit
		if (s_camera3D == nullptr)
		{
			return;
		}


		MeshBuilder builder = MeshBuilder();
		builder.Initialize(Renderer::PRIMITIVE_LINES, false);

		builder.SetVertexColor(GetCurrentColor());

		// Front face
		builder.AddLine(m_projectionPoints[0], m_projectionPoints[1]);
		builder.AddLine(m_projectionPoints[1], m_projectionPoints[2]);
		builder.AddLine(m_projectionPoints[2], m_projectionPoints[3]);
		builder.AddLine(m_projectionPoints[3], m_projectionPoints[0]);


		// Back face
		builder.AddLine(m_projectionPoints[4], m_projectionPoints[5]);
		builder.AddLine(m_projectionPoints[5], m_projectionPoints[6]);
		builder.AddLine(m_projectionPoints[6], m_projectionPoints[7]);
		builder.AddLine(m_projectionPoints[7], m_projectionPoints[4]);


		// Connecting lines
		builder.AddLine(m_projectionPoints[0], m_projectionPoints[4]);
		builder.AddLine(m_projectionPoints[1], m_projectionPoints[5]);
		builder.AddLine(m_projectionPoints[2], m_projectionPoints[6]);
		builder.AddLine(m_projectionPoints[3], m_projectionPoints[7]);


		builder.Finalize();
		Mesh* mesh = builder.CreateMesh();

		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(m_transform.GetLocalToWorldMatrix());
		g_theRenderer->DrawMesh(mesh->GetSubMesh(0));
		delete mesh;
	}

	Transform m_transform;
	std::vector<Vector3> m_projectionPoints;

	Material* m_material = nullptr;
};

class DebugRenderTask_Glyph : public DebugRenderTask
{
public:
	DebugRenderTask_Glyph(float lifespan, const Vector3& position, const AABB2D& screenDimensions, const Texture* texture, const RGBA& startTint, const RGBA& endTint) 
		: m_worldPosition(position)
		, m_screenDimensions(screenDimensions)
	{

		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startTint;
		m_options.m_endColor	= endTint;

		m_material = Material::Get("DebugDraw_2D");
		if (texture != nullptr)
		{
			m_material->SetDiffuseTexture(texture);
		}
	};

	~DebugRenderTask_Glyph()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const 
	{
		// Short Circuit
		if (s_camera3D == nullptr)
		{
			return;
		}

		// If we cant see the point just dont
		Vector3 dirToPos = m_worldPosition - s_camera3D->GetPosition();
		dirToPos.NormalizeAndGetLength();
		if (DotProduct(dirToPos, s_camera3D->GetForward()) < 0.0f)
		{
			return;
		}

		RGBA color = GetCurrentColor();
		Vector2 screenPosition = Vector2(s_camera3D->WorldToScreen(m_worldPosition));
		AABB2D screenBounds = m_screenDimensions + screenPosition;
		
		Camera* ssCamera = s_camera3D->CreateScreenSpaceCameraForViewport();
		g_theRenderer->BindCamera(ssCamera);
		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(Matrix4());

		Mesh* quad = CreateAABB2D(screenBounds, color);
		g_theRenderer->DrawMesh(quad->GetSubMesh(0));
		delete quad;

		g_theRenderer->BindCamera(s_camera2D);
		delete ssCamera;
		ssCamera = nullptr;
	};

	Vector3 m_worldPosition;
	AABB2D m_screenDimensions;

	Material* m_material = nullptr;

};

class DebugRenderTask_TextTag : public DebugRenderTask
{
public:
	DebugRenderTask_TextTag(float lifespan, const Vector3& position, float fontSize, const RGBA& startColor, const RGBA& endColor, const std::string& text )
		: m_worldPosition(position)
		, m_fontSize(fontSize)
		, m_text(text)
	{
		m_options.m_lifespan	= lifespan;
		m_options.m_startColor	= startColor;
		m_options.m_endColor	= endColor;

		float textWidth = s_font->GetStringWidth(m_text, m_fontSize, m_aspect);
		m_screenBounds.mins.x = -textWidth  * 0.5f;
		m_screenBounds.mins.y = -m_fontSize * 0.5f;
		m_screenBounds.maxs.x = +textWidth  * 0.5f;
		m_screenBounds.maxs.y = +m_fontSize * 0.5f;

		m_material = Material::Get("DebugDraw_2D");
	};

	~DebugRenderTask_TextTag()
	{
		delete m_material;
		m_material = nullptr;
	}

	void Render() const 
	{
		// Short Circuit
		if (s_camera3D == nullptr)
		{
			return;
		}

		// If we cant see the point just dont
		Vector3 dirToPos = m_worldPosition - s_camera3D->GetPosition();
		dirToPos.NormalizeAndGetLength();
		if (DotProduct(dirToPos, s_camera3D->GetForward()) < 0.0f)
		{
			return;
		}


		Vector2 screenPosition = Vector2(s_camera3D->WorldToScreen(m_worldPosition));
		AABB2D screenBounds = m_screenBounds + screenPosition;

		Camera* ssCamera = s_camera3D->CreateScreenSpaceCameraForViewport();
		g_theRenderer->BindCamera(ssCamera);
		g_theRenderer->BindMaterial(m_material);
		g_theRenderer->BindModelMatrix(Matrix4());

		RGBA color = GetCurrentColor();
		g_theRenderer->DrawText2D(screenBounds.mins, m_text, m_fontSize, color, m_aspect, s_font);

		g_theRenderer->BindCamera(s_camera2D);
		delete ssCamera;
		ssCamera = nullptr;
	}

	Vector3		m_worldPosition;
	std::string m_text;
	float		m_fontSize;
	float		m_aspect = 1.0f;

	AABB2D		m_screenBounds;

	Material*	m_material = nullptr;
};



// -----------------------------------------------------------------------------------------------------------------------------------------------
// -- Debug Draw API -----------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------
Mesh* CreatePointMesh()
{
	// Build Mesh
	MeshBuilder builder = MeshBuilder();
	builder.Initialize(Renderer::PRIMITIVE_LINES, false);

	builder.SetVertexColor(RGBA());
	builder.SetVertexUVs(0.0f, 0.0f);

	// X bar
	builder.PushVertex(-Vector3(0.1f, 0.0f, 0.0f));
	builder.PushVertex( Vector3(0.1f, 0.0f, 0.0f));

	// Y bar
	builder.PushVertex(-Vector3(0.0f, 0.1f, 0.0f));
	builder.PushVertex( Vector3(0.0f, 0.1f, 0.0f));

	// Z bar
	builder.PushVertex(-Vector3(0.0f, 0.0f, 0.1f));
	builder.PushVertex( Vector3(0.0f, 0.0f, 0.1f));

	builder.Finalize();
	Mesh* pointMesh = builder.CreateMesh();
	return pointMesh;
}


// ----------------------------------------------------------------------------------------
Mesh* CreateBasisMesh()
{
	// Build Mesh
	MeshBuilder builder = MeshBuilder();
	builder.Initialize(Renderer::PRIMITIVE_LINES, false);

	builder.SetVertexUVs(0.0f, 0.0f);

	// right
	builder.SetVertexColor(RGBA(255,0,0));
	builder.PushVertex(Vector3::ZEROS);
	builder.PushVertex(Vector3::ZEROS + Vector3::RIGHT);

	// up
	builder.SetVertexColor(RGBA(0,255,0));
	builder.PushVertex(Vector3::ZEROS);
	builder.PushVertex(Vector3::ZEROS + Vector3::UP);

	// forward
	builder.SetVertexColor(RGBA(0,0,255));
	builder.PushVertex(Vector3::ZEROS);
	builder.PushVertex(Vector3::ZEROS + Vector3::FORWARD);

	builder.Finalize();
	Mesh* mesh = builder.CreateMesh();
	return mesh;
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Initialize()
{
	// Visibility
	s_render = true;
	

	// Task lists
	s_debugRenderTasks_2D.clear();
	s_debugRenderTasks_3D_ToScreen.clear();
	s_debugRenderTasks_3D.clear();
	

	// 3D camera
	s_camera3D = nullptr;


	// 2D camera
	s_camera2D = new Camera();
	s_camera2D->SetColorTarget(g_theRenderer->GetDefaultColorTarget());
	s_camera2D->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	s_camera2D->SetProjection(Matrix4::MakeOrtho2D(Vector2::ZEROS, (Vector2)g_theWindow->GetDimensions()));


	// Font
	s_font = BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont18.png");


	// Shaders
	s_debugShader_2D		 = Shader::Get("DebugDraw_2D");
	s_debugShader_3D_depth	 = Shader::Get("DebugDraw_3D_depth");
	s_debugShader_3D_noDepth = Shader::Get("DebugDraw_3D_noDepth");


	// Textures
	s_defaultTexture = Texture::CreateOrGet("Data/BuiltIns/DEFAULT_TEXTURE.png", Texture::TEXTURE_FORMAT_RGBA8);


	// Meshes
	s_pointMesh		= CreatePointMesh();
	s_basisMesh		= CreateBasisMesh();
	s_sphereMesh	= CreateWireSphere(Vector3::ZEROS, 1.0f, 10, 10, RGBA());
	s_cubeMesh		= CreateWireCube(Vector3::ZEROS, Vector3::ONES, RGBA()); 


	// Log
	float logPad = 8.0f;
	s_logAncor = Vector2(logPad, g_theWindow->GetDimensions().y - logPad);
	s_log.clear();


	// Console commands
	RegisterCommand("DebugDrawClear", DebugRenderClearRenderCommands_Command);
	RegisterCommand("DebugDrawSetVisibility", DebugRenderSetVisibility_Command);
	RegisterCommand("DebugDrawToggleGrid", DebugRenderToggleGrid_Command);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Destroy()
{
	s_debugRenderTasks_2D.clear();
	s_debugRenderTasks_3D.clear();
	s_camera3D = nullptr;
	s_font = nullptr;


	// Log
	RemoveAllTasks(s_log);


	// Meshes
	delete s_pointMesh;
	s_pointMesh = nullptr;

	delete s_basisMesh;
	s_basisMesh = nullptr;

	delete s_sphereMesh;
	s_sphereMesh = nullptr;

	delete s_cubeMesh;
	s_cubeMesh = nullptr;


	// Textures
	s_defaultTexture = nullptr;


	// Shaders
	s_debugShader_2D		 = nullptr;
	s_debugShader_3D_depth	 = nullptr;
	s_debugShader_3D_noDepth = nullptr;


	// Font
	s_font = nullptr;


	// 2D camera
	delete s_camera2D;
	s_camera2D = nullptr;


	// 3D camera
	s_camera3D = nullptr;


	// Task lists
	RemoveAllTasks(s_debugRenderTasks_2D);
	RemoveAllTasks(s_debugRenderTasks_3D_ToScreen);
	RemoveAllTasks(s_debugRenderTasks_3D);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_BeginFrame()
{
	if (s_drawGrid)
	{
		DebugRenderTask_Grid* task = new DebugRenderTask_Grid(0.0f, Vector3((float)s_numGridLines, 0.0f, (float)s_numGridLines), (float)s_numGridLines, s_majorLineEvery, s_majorLineColor, s_minorLineColor);
		s_debugRenderTasks_3D.push_back(task);
	}
}


// ----------------------------------------------------------------------------------------
void DebugDraw_EndFrame()
{
	DebugDraw_RemoveAllCompletedTasks();
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Update()
{
	float deltaSeconds = g_theMasterClock->GetDeltaSeconds();

	for (size_t i = 0; i < s_log.size(); ++i)
	{
		DebugRenderTask* task = s_log[i];
		task->Update(deltaSeconds);
	}

	for (size_t i = 0; i < s_debugRenderTasks_2D.size(); ++i)
	{
		DebugRenderTask* task = s_debugRenderTasks_2D[i];
		task->Update(deltaSeconds);
	}

	for (size_t i = 0; i < s_debugRenderTasks_3D.size(); ++i)
	{
		DebugRenderTask* task = s_debugRenderTasks_3D[i];
		task->Update(deltaSeconds);
	}

	for (size_t i = 0; i < s_debugRenderTasks_3D_ToScreen.size(); ++i)
	{
		DebugRenderTask* task = s_debugRenderTasks_3D_ToScreen[i];
		task->Update(deltaSeconds);
	}
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Render2D()
{
	PROFILE_SCOPE_FUNCTION();

	// Short Circuit
	if(s_render == false)
	{
		return;
	}

	g_theRenderer->BindCamera(s_camera2D);
	for (size_t i = 0; i < s_debugRenderTasks_2D.size(); ++i)
	{
		DebugRenderTask* task = s_debugRenderTasks_2D[i];
		task->Render();
	}

	DebugDraw_RenderLog();
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Render3DForCamera(Camera* camera)
{
	PROFILE_SCOPE_FUNCTION();

	// Short Circuit
	if(s_render == false)
	{
		return;
	}

	// Short Circuit
	if (camera == nullptr)
	{
		return;
	}



	// Render all 3D tasks
	s_camera3D = camera;
	g_theRenderer->BindCamera(camera);
	for (size_t i = 0; i < s_debugRenderTasks_3D.size(); ++i)
	{
		DebugRenderTask* task = s_debugRenderTasks_3D[i];

		if (task->m_options.m_mode == DEBUG_RENDER_IGNORE_DEPTH)
		{
			g_theRenderer->BindShader(s_debugShader_3D_noDepth);
		}
		else
		{
			g_theRenderer->BindShader(s_debugShader_3D_depth);
		}

		task->Render();
	}



	// Draw Text Tags, Glyphs, etc... (3D location w/ screen space draw)
	for (size_t i = 0; i < s_debugRenderTasks_3D_ToScreen.size(); ++i)
	{
		DebugRenderTask* task = s_debugRenderTasks_3D_ToScreen[i];
		task->Render();
	}
}


// ----------------------------------------------------------------------------------------
void DebugDraw_RenderLog()
{
	PROFILE_SCOPE_FUNCTION();

	// Short Circuit
	if(s_render == false)
	{
		return;
	}

	g_theRenderer->BindCamera(s_camera2D);
	Vector2 currentDrawPoint = s_logAncor;
	for (int i = 0; i < (int)s_log.size(); ++i)
	{
		DebugRenderTask_Text* task = (DebugRenderTask_Text*)s_log[i];
		currentDrawPoint.y -= s_logFontSize;

		task->m_lowerLeft = currentDrawPoint;
		task->Render();
	}
}


// ----------------------------------------------------------------------------------------
void RemoveCompletedTasks(std::vector<DebugRenderTask*>& tasks)
{
	for (size_t i = 0; i < tasks.size(); /*none*/)
	{
		DebugRenderTask* task = tasks[i];
		bool canBeRemoved = task->CanBeRemoved();
		if(canBeRemoved)
		{
			// Remove this task
			delete task;
			tasks[i] = nullptr;

			size_t tasksSize = tasks.size();
			if (tasksSize >= 2)
			{
				// Take the last task and put it in the current ones spot in the vector
				tasks[i] = tasks[tasksSize - 1];
				tasks.erase(tasks.begin() + (tasksSize - 1));
			}
			else
			{
				tasks.erase(tasks.begin() + i);
			}
		}
		else
		{
			++i;
		}
	}
}


// ----------------------------------------------------------------------------------------
void RemoveCompletedLogTasks(std::vector<DebugRenderTask*>& tasks)
{
	for (int i = tasks.size() - 1; i >= 0; --i)
	{
		DebugRenderTask* task = tasks[i];
		bool canBeRemoved = task->CanBeRemoved();
		if(canBeRemoved)
		{
			tasks.erase(tasks.begin() + i);
		}
	}
}

// ----------------------------------------------------------------------------------------
void RemoveAllTasks(std::vector<DebugRenderTask*>& tasks)
{
	for (int i = tasks.size() - 1; i >= 0; --i)
	{
		tasks.erase(tasks.begin() + i);
	}
	tasks.clear();
}


// ----------------------------------------------------------------------------------------
void DebugDraw_RemoveAllCompletedTasks()
{
	RemoveCompletedLogTasks(s_log);
	RemoveCompletedTasks(s_debugRenderTasks_2D);
	RemoveCompletedTasks(s_debugRenderTasks_3D);
	RemoveCompletedTasks(s_debugRenderTasks_3D_ToScreen);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_SetVisibility(bool visibility)
{
	s_render = visibility;
}


// ----------------------------------------------------------------------------------------
void DebugDraw_ClearCommands()
{
	s_debugRenderTasks_2D.clear();
	s_debugRenderTasks_3D.clear();
}


// ----------------------------------------------------------------------------------------
void DebugDraw_SetGridVisibility(bool visibility)
{
	s_drawGrid = visibility;
}


// ----------------------------------------------------------------------------------------
void DebugDraw_DrawGrid(unsigned int numGridLines, unsigned int majorLineEvery, const RGBA& majorLineColor, const RGBA& minorLineColor)
{
	s_numGridLines		= numGridLines;
	s_majorLineEvery	= majorLineEvery;
	s_majorLineColor	= majorLineColor;
	s_minorLineColor	= minorLineColor;
}



// -----------------------------------------------------------------------------------------------------------------------------------------------
// -- Logging ------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------
void DebugDraw_Log( float lifespan, char const *format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringf(format, variableArgumentList);
	va_end( variableArgumentList );

	DebugRenderTask_Text* task = new DebugRenderTask_Text(lifespan, s_logAncor, s_logFontSize, RGBA(), RGBA(), text);
	s_log.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Log( float lifespan, const RGBA& color, char const *format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringf(format, variableArgumentList);
	va_end( variableArgumentList );

	DebugRenderTask_Text* task = new DebugRenderTask_Text(lifespan, s_logAncor, s_logFontSize, color, color, text);
	s_log.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Log( float lifespan, const RGBA& startColor, const RGBA& endColor, char const *format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringf(format, variableArgumentList);
	va_end( variableArgumentList );

	DebugRenderTask_Text* task = new DebugRenderTask_Text(lifespan, s_logAncor, s_logFontSize, startColor, endColor, text);
	s_log.push_back(task);
}



// -----------------------------------------------------------------------------------------------------------------------------------------------
// -- 2D Adders ----------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------
void DebugDraw_2DQuad( float lifespan, const AABB2D& bounds, const Texture* texture, const RGBA& startColor, const RGBA& endColor )
{
	DebugRenderTask_Quad* task = new DebugRenderTask_Quad(lifespan, bounds, texture, startColor, endColor);
	s_debugRenderTasks_2D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_2DLine( float lifespan, const Vector2& point0, const RGBA& point0Color, const Vector2& point1, const RGBA& point1Color, const RGBA& startTint, const RGBA& endTint )
{
	DebugRenderTask_Line* task = new DebugRenderTask_Line(lifespan, point0, point0Color, point1, point1Color, startTint, endTint);
	s_debugRenderTasks_2D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_2DText( float lifespan, const Vector2& position, float fontSize, const RGBA& startColor, const RGBA& endColor, char const *format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringf(format, variableArgumentList);
	va_end( variableArgumentList );

	DebugRenderTask_Text* task = new DebugRenderTask_Text(lifespan, position, fontSize, startColor, endColor, text);
	s_debugRenderTasks_2D.push_back(task);
}




// -----------------------------------------------------------------------------------------------------------------------------------------------
// -- 3D Adders ----------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------
void DebugDraw_Point( float lifespan, const Vector3& position, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode )
{
	DebugRenderTask_Point* task = new DebugRenderTask_Point(lifespan, position, startColor, endColor, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_LineSegment( float lifespan, const Vector3& point0, const RGBA& point0Color, const Vector3& point1, const RGBA& point1Color, const RGBA& startTint, const RGBA& endTint, DebugRenderMode const mode )
{
	DebugRenderTask_LineSegment* task = new DebugRenderTask_LineSegment(lifespan, point0, point0Color, point1, point1Color, startTint, endTint, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Basis( float lifespan, const Matrix4& basis, const RGBA& startTint, const RGBA& endTint, DebugRenderMode const mode )
{
	DebugRenderTask_Basis* task = new DebugRenderTask_Basis(lifespan, basis, startTint, endTint, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_WireSphere( float lifespan, const Vector3& position, float radius, const RGBA& startColor, const RGBA& endColor, DebugRenderMode const mode )
{
	DebugRenderTask_WireSphere* task = new DebugRenderTask_WireSphere(lifespan, position, radius, startColor, endColor, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_WireAABB3( float lifespan, const Vector3& position, const Vector3& halfDimensions, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode )
{
	DebugRenderTask_WireAABB3* task = new DebugRenderTask_WireAABB3(lifespan, position, halfDimensions, startColor, endColor, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_TexturedQuad( float lifespan, Vector3 const &position, Vector3 const &right, Vector3 const &up, const Vector2& halfDimensions, const Texture* texture, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode )
{
	DebugRenderTask_TexturedQuad* task = new DebugRenderTask_TexturedQuad(lifespan, position, right, up, halfDimensions, texture, startColor, endColor, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_3DText(float lifespan, const Vector3& position, const Vector2& pivot, const Vector3& right, const Vector3& up, float fontSize, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode, char const *format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringf(format, variableArgumentList);
	va_end( variableArgumentList );

	DebugRenderTask_3DText* task = new DebugRenderTask_3DText(lifespan, position, pivot, right, up, fontSize, startColor, endColor, text, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_3DTextBillboarded(float lifespan, const Vector3& position, const Vector2& pivot, float fontSize, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode, char const *format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringf(format, variableArgumentList);
	va_end( variableArgumentList );

	DebugRenderTask_3DText* task = new DebugRenderTask_3DText(lifespan, position, pivot, Vector3::ZEROS, Vector3::ZEROS, fontSize, startColor, endColor, text, mode, true);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Frustum(float lifespan, const Matrix4& projection, const Matrix4& cameraBasis, const RGBA& startColor, const RGBA& endColor, DebugRenderMode mode)
{
	DebugRenderTask_Frustum* task = new DebugRenderTask_Frustum(lifespan, projection, cameraBasis, startColor, endColor, mode);
	s_debugRenderTasks_3D.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_TextTag( float lifespan, const Vector3& position, float fontSize, const RGBA& startColor, const RGBA& endColor, char const *format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringf(format, variableArgumentList);
	va_end( variableArgumentList );

	DebugRenderTask_TextTag* task = new DebugRenderTask_TextTag(lifespan, position, fontSize, startColor, endColor, text);
	s_debugRenderTasks_3D_ToScreen.push_back(task);
}


// ----------------------------------------------------------------------------------------
void DebugDraw_Glyph( float lifespan, const Vector3& position, const AABB2D& dimensions, const Texture* texture, const RGBA& startTint, const RGBA& endTint)
{
	DebugRenderTask_Glyph* task = new DebugRenderTask_Glyph(lifespan, position, dimensions, texture, startTint, endTint);
	s_debugRenderTasks_3D_ToScreen.push_back(task);
}
