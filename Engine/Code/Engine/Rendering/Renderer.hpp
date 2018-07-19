#pragma once
#pragma comment( lib, "opengl32" )

#include <string>
#include <list>

#include "Engine/Core/RGBA.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/AABB2D.hpp"

#include "Engine/Rendering/RenderBuffer.h"
#include "Engine/Rendering/UniformBuffer.hpp"
#include "Engine/Rendering/Shader.hpp"

class	FrameBuffer;
class	LightData;
class	Light;
class	Camera;
class	ShaderProgram;
struct	RenderState;
class	Shader;
class	Texture;
class	TextureCube;
class	Sampler;
class	Material;
struct	DrawInstruction;
class	VertexLayout;
class	SubMesh;
class	BitmapFont;
class	Sprite;
class	IsoSprite;



enum TextDrawMode
{
	TEXT_DRAW_INVALID,

	TEXT_DRAW_OVERRUN,
	TEXT_DRAW_WORD_WRAP,
	TEXT_DRAW_SHRINK_TO_FIT,

	TEXT_DRAW_COUNT
};



class Renderer
{
public:
	// ----------------------------------------------------------------------------------------------------------------
	// -- Composition
	// ----------------------------------------------------------------------------------------------------------------
	Renderer() {};
	~Renderer() {};

	void			Initialize();
	void			Destroy();



	// ----------------------------------------------------------------------------------------------------------------
	// -- Upkeep
	// ----------------------------------------------------------------------------------------------------------------
	void			BeginFrame();
	void			EndFrame();
	


	// ----------------------------------------------------------------------------------------------------------------
	// -- Lighting
	// ----------------------------------------------------------------------------------------------------------------
	// Ambient
	void			SetAmbientLight(const RGBA& color, float brightness); // brightness [0 - 1]

	// Directional/ Sun
	void			SetShadowCascadeBreakpoints(const Camera* camera);
	void			SetSunLight(const Light* light, bool setShadowMap = true);
	void			DisableSunLight();

	// Punctual Lights
	void			SetPunctualLight(unsigned int index, const Light* light, bool setShadowMap = true);
	void			DisableAllPunctualLights();
	
	// Punctual Light Data
	void			SetLightData(unsigned int index, const LightData* light);
	void			SetPointLightData(unsigned int index, const Vector3& position, const RGBA& color = RGBA(), float intensity = 1.0f, const Vector3& attenuation = Vector3(0.0f, 0.0f, 1.0f));
	void			SetDirectionalLightData(unsigned int index, const Vector3& position, const Vector3& direction, const RGBA& color = RGBA(), float intensity = 1.0f, const Vector3& attenuation = Vector3(0.0f, 0.0f, 1.0f));
	void			SetConeLightData(unsigned int index, const Vector3& position, const Vector3& direction, float innerAngle, float outerAngle, const RGBA& color = RGBA(), float intensity = 1.0f, const Vector3& attenuation = Vector3(0.0f, 0.0f, 1.0f));

	// Punctual Shadow Map Data
	void			SetShadowMapData(unsigned int index, const Matrix4& viewProjection);



	// ----------------------------------------------------------------------------------------------------------------
	// -- State Management
	// ----------------------------------------------------------------------------------------------------------------
	// Flags
	void			EnableCapability(unsigned int capability);
	void			DisableCapability(unsigned int capability);

	// Viewport
	void			SetViewport(const Vector2& lowerLeft, const Vector2& upperRight);

	// Buffer Swap
	void			SwapScreenBuffers();

	// Clearing
	void			ClearColor(const RGBA& color = RGBA(0,0,0));
	void			ClearDepth(float depth = 1.0f);

	// Camera
	void			BindCamera(Camera* camera);

	// Material
	void			BindMaterial(const Material* material);

	// Material -> Shader
	void			BindShader(const Shader* shader);
	void			BindShaderProgram(const ShaderProgram* shaderProgram);
	void			BindRenderState(const RenderState& renderState);

	// Material -> Textures
	void			BindTextureAndSampler(unsigned int bindPoint, const Texture* texture, const Sampler* sampler);
	void			BindTexture(unsigned int bindPoint, const Texture* texture);
	void			BindSampler(unsigned int bindPoint, const Sampler* sampler);
	void			BindTextureCube(unsigned int bindPoint, const TextureCube* textureCube);

	// Model Matrix
	void			BindModelMatrix(const Matrix4& modelMatrix);

	// Mesh
	void			BindMeshToProgram(const Shader* shader, const SubMesh* mesh);

	// Mesh -> Array Buffers
	void			BindArrayBuffer(unsigned int target, RenderBuffer& arrayBuffer);
	void			BindVertexBuffer(const RenderBuffer& vertexBuffer);
	void			BindIndexBuffer(const RenderBuffer& indexBuffer);

	// Mesh -> Vertex Layout
	void			BindVertexLayoutToShaderProgram(const VertexLayout* vertexLayout, const ShaderProgram* shaderProgram);
	
	// Uniform Blocks	
	void			BindUniformBlock(UniformBlock& uniformBlock, unsigned int blockBindPoint);
	


	// ----------------------------------------------------------------------------------------------------------------
	// -- Drawing
	// ----------------------------------------------------------------------------------------------------------------
	// Low Level Drawing
	void			DrawIndexed(const DrawInstruction& drawInstruction);
	void			DrawArrays(const DrawInstruction& drawInstruction);

	// High Level Drawing
	void			DrawMesh(const SubMesh* mesh);
	void			DrawMeshImmediate(const Vertex_3DPCU* verts, int numVerts, const unsigned int* indices, int numIndices, unsigned int drawPrimitive);



	// ----------------------------------------------------------------------------------------------------------------
	// -- Screen Effects
	// ----------------------------------------------------------------------------------------------------------------
	void			ApplyEffect(const Material* material);
	void			FinishEffects();

	

	// ----------------------------------------------------------------------------------------------------------------
	// -- FrameBuffer RenderTargets 
	// ----------------------------------------------------------------------------------------------------------------
	bool			CopyFrameBuffer(FrameBuffer* to, FrameBuffer* from);
	Texture*		GetDefaultColorTarget() const;
	Texture*		GetDefaultDepthStencilTarget() const;



	// ----------------------------------------------------------------------------------------------------------------
	// -- Utility Drawing
	// ----------------------------------------------------------------------------------------------------------------
	// Sprites
	void			DrawSprite(const Sprite* sprite, const RGBA& tint, const Vector3& position, const Vector3& right, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f), const Vector2& scale = Vector2(1.0f, 1.0f));
	void			DrawIsoSprite(const IsoSprite* isoSprite, const RGBA& tint, const Vector3& spritePosition, const Vector3& spriteForward, const Vector3& spriteRight, const Vector3& spriteUp, const Vector3& viewerForward, const Vector3& viewerRight, const Vector2& scale = Vector2(1.0f, 1.0f));

	// Text Drawing
	void			DrawText2D(const Vector2& drawMins, const std::string& asciiText, float cellHeight, RGBA tint, float aspectScale, const BitmapFont* font, float percentageComplete = 1.0f);
	void			DrawTextInBox2D(const AABB2D& bounds, const std::string& asciiText, float cellHeight, RGBA tint, float aspectScale, const BitmapFont* font, const Vector2& alignment, TextDrawMode drawMode, float percentageComplete = 1.0f);
	void			DrawText3D(const Vector3& position, const Vector3& right, const Vector3& up, const std::string& asciiText, float cellHeight, RGBA tint, float aspectScale, const BitmapFont* font, float percentageComplete = 1.0f);
	void			DrawTextInBox3D(const Vector3& position, const Vector2& dimensions, const Vector2& pivot, const Vector3& right, const Vector3& up, const std::string& asciiText, float cellHeight, RGBA tint, float aspectScale, const BitmapFont* font, const Vector2& alignment, TextDrawMode drawMode, float percentageComplete = 1.0f);

	// Text Utilities
	void			WordWrapLines(std::list<char*>& lines, float boxWidth, float cellHeight, float aspectScale, const BitmapFont* font);



	// ----------------------------------------------------------------------------------------------------------------
	// -- ENUMS
	// ----------------------------------------------------------------------------------------------------------------
	// Renderer Capabilities
	static const unsigned int CAPABILITY_BACKFACE_CULLING;
	static const unsigned int CAPABILITY_BLEND;
	static const unsigned int CAPABILITY_LINE_SMOOTH;

	// Renderer Draw Primitives
	static const unsigned int PRIMITIVE_LINES;
	static const unsigned int PRIMITIVE_TRIANGLES;
	static unsigned int		  StringToPrimitiveType(const std::string& name, unsigned int def = PRIMITIVE_TRIANGLES);

	// Polygon Fill Modes
	static const unsigned int FILL_SOLID;
	static const unsigned int FILL_WIREFRAME;
	static unsigned int		  StringToFillMode(const std::string& name, unsigned int def = FILL_SOLID);

	// Face Culling
	static const unsigned int CULL_FRONT;
	static const unsigned int CULL_BACK;
	static const unsigned int CULL_NONE;
	static unsigned int		  StringToCullMode(const std::string& name, unsigned int def = CULL_NONE);

	// Winding Order
	static const unsigned int WIND_CW;
	static const unsigned int WIND_CCW;
	static unsigned int		  StringToWindingOrder(const std::string& name, unsigned int def = WIND_CCW);

	// Blend Ops
	static const unsigned int BLEND_OP_ADD;
	static const unsigned int BLEND_OP_SUBTRACT;
	static const unsigned int BLEND_OP_REVSUB;
	static const unsigned int BLEND_OP_MIN;
	static const unsigned int BLEND_OP_MAX;
	static unsigned int		  StringToBlendOp(const std::string& name, unsigned int def = BLEND_OP_ADD);

	// Blending Factors
	static const unsigned int BLEND_FACTOR_ZERO;
	static const unsigned int BLEND_FACTOR_ONE;
	static const unsigned int BLEND_FACTOR_SOURCE_COLOR;
	static const unsigned int BLEND_FACTOR_ONE_MINUS_SOURCE_COLOR;
	static const unsigned int BLEND_FACTOR_SOURCE_ALPHA;
	static const unsigned int BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA;
	static const unsigned int BLEND_FACTOR_DESTINATION_COLOR;
	static const unsigned int BLEND_FACTOR_ONE_MINUS_DESTINATION_COLOR;
	static const unsigned int BLEND_FACTOR_DESTINATION_ALPHA;
	static const unsigned int BLEND_FACTOR_ONE_MINUS_DESTINATION_ALPHA;
	static const unsigned int BLEND_FACTOR_SOURCE_ALPHA_SATURATE;
	static unsigned int		  StringToBlendFactor(const std::string& name, unsigned int def = BLEND_FACTOR_ZERO);

	// Depth Comparison Functions
	static const unsigned int DEPTH_COMPARE_NEVER;    
	static const unsigned int DEPTH_COMPARE_LESS;        
	static const unsigned int DEPTH_COMPARE_LEQUAL;      
	static const unsigned int DEPTH_COMPARE_GREATER;     
	static const unsigned int DEPTH_COMPARE_GEQUAL;      
	static const unsigned int DEPTH_COMPARE_EQUAL;       
	static const unsigned int DEPTH_COMPARE_NOT_EQUAL;   
	static const unsigned int DEPTH_COMPARE_ALWAYS;      
	static unsigned int		  StringToDepthTest(const std::string& name, unsigned int def = DEPTH_COMPARE_ALWAYS);

	// Texturing
	static const unsigned int TEXTURE_2D;

	// FrameBuffer Targets
	static const unsigned int TEXTURE_FORMAT_RGBA8; // default color format
	static const unsigned int TEXTURE_FORMAT_D24S8; 

	// UniformBuffer Bind Points
	static const unsigned int TIME_BUFFER_BIND_POINT;
	static const unsigned int CAMERA_BUFFER_BIND_POINT;
	static const unsigned int MODEL_BUFFER_BIND_POINT;
	static const unsigned int LIGHT_BUFFER_BIND_POINT;
	static const unsigned int CLEAR_BUFFER_BIND_POINT;
	static const unsigned int FOG_BUFFER_BIND_POINT;
	static const unsigned int MATERIAL_PROPERTY_BUFFER_0_BIND_POINT;

	// Texture Bind Points
	static const unsigned int DIFFUSE_TEXTURE_BIND_POINT;
	static const unsigned int NORMAL_MAP_BIND_POINT;
	static const unsigned int SKYBOX_BIND_POINT;
	static const unsigned int SHADOW_MAP_BIND_POINT_0;
	static const unsigned int SUN_CASCADE_BIND_POINT_0;



private:
	// ----------------------------------------------------------------------------------------------------------------
	// -- Pipeline
	// ----------------------------------------------------------------------------------------------------------------
	// Time
	UniformBlock	m_timeBlock;
	
	// Clear
	Shader			m_clearShader;
	UniformBlock	m_clearBlock;

	// Lighting
	UniformBlock    m_lightBlock;

	// Model
	UniformBlock	m_modelBlock;

	// Fog
	UniformBlock	m_fogBlock;

	// Mesh
	unsigned int	m_defaultVAO;
	const Shader*	m_currentShader = nullptr;

	// RenderTargets
	Texture*		m_defaultColorTarget	= nullptr;
	Texture*		m_defaultDepthTarget	= nullptr;
	Camera*			m_defaultCamera			= nullptr;  // Really only for the frame buffer

	// Screen effects
	Texture*		m_screenEffectTarget; 
	Texture*		m_screenEffectScratchTarget; 
	Camera*			m_screenEffectsCamera;				// setup with VIEW and PERSPECTIVE as Identity


	
	// ----------------------------------------------------------------------------------------------------------------
	// -- Draw Utility
	// ----------------------------------------------------------------------------------------------------------------
	// DrawMeshImmediate State
	RenderBuffer	m_immediateVertexBuffer;
	RenderBuffer	m_immediateIndexBuffer;
};