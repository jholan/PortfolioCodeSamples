#include "Engine/Rendering/Renderer.hpp"

#include <string>

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Engine/Rendering/GLFunctions.h"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/Sampler.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/Sprite.hpp"
#include "Engine/Rendering/IsoSprite.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/Light.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Rendering/TextureCube.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Clock.hpp"

extern Blackboard	g_gameConfigBlackboard;
extern Window*		g_theWindow;
extern Clock*		g_theMasterClock;
extern Clock*		g_theGameClock;


//-------------------------------------------------------------------------------------------------------
static HMODULE	gGLLibrary  = NULL; 
static HWND		gGLwnd      = NULL;    // window our context is attached to; 
static HDC		gHDC        = NULL;    // our device context
static HGLRC	gGLContext  = NULL;    // our rendering context; 



#define MAX_LIGHTS 8



struct ShadowMapData
{
	Matrix4  m_viewProjection;
};


struct CascadeBreakpoint
{
	float m_breakpoint;
	float m_cb_padding_0;
	float m_cb_padding_1;
	float m_cb_padding_2;
};


struct LightUniformBlock 
{
	Vector4			m_ambient;

	// Sun
	LightData		m_sun;
	ShadowMapData	m_sunCascades[NUM_CASCADES];

	CascadeBreakpoint m_cascadeBreakpoints[NUM_CASCADES];

	// For point, directional, spot
	LightData		m_lights[MAX_LIGHTS];
	ShadowMapData	m_shadowMaps[MAX_LIGHTS];
}; 



struct ModelUniformBlock 
{
	Matrix4 m_model;
}; 



struct ClearUniformBlock
{
	Vector4		CLEAR_COLOR; 
	float		DEPTH_TO_CLEAR_TO;
	float		SHOULD_CLEAR_DEPTH; // 0 for yes, 1 for no
};



struct TimeUniformBlock
{
	float		m_systemLifetime;
	float		m_gameLifetime;
	float		m_sytstemDeltaSeconds;
	float		m_gameDeltaSeconds;
};



//-------------------------------------------------------------------------------------------------------
static HGLRC CreateDummyRenderContext( HDC hdc ) 
{
	// Setup the output to be able to render how we want
	// (in our case, an RGBA (4 bytes per channel) output that supports OpenGL
	// and is double buffered
	PIXELFORMATDESCRIPTOR pfd;
	memset( &pfd, 0, sizeof(pfd) ); 
	pfd.nSize = sizeof(pfd);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0;		// 24; Depth/Stencil handled by FBO
	pfd.cStencilBits = 0;	// 8; DepthStencil handled by FBO
	pfd.iLayerType = PFD_MAIN_PLANE; // ignored now according to MSDN

	// Find a pixel format that matches our search criteria above. 
	int pixel_format = ChoosePixelFormat( hdc, &pfd );
	if ( pixel_format == NULL ) 
	{
		return NULL; 
	}

	// Set our HDC to have this output. 
	if (!SetPixelFormat( hdc, pixel_format, &pfd )) 
	{
		return NULL; 
	}

	// Create the context for the HDC
	HGLRC context = wglCreateContext( hdc );
	if (context == NULL) 
	{
		return NULL; 
	}

	// return the context; 
	return context; 
}


//-------------------------------------------------------------------------------------------------------
void BindGLContextCreationFunctions()
{
	GL_BIND_FUNCTION( wglGetExtensionsStringARB ); 
	GL_BIND_FUNCTION( wglChoosePixelFormatARB ); 
	GL_BIND_FUNCTION( wglCreateContextAttribsARB );
}


//-------------------------------------------------------------------------------------------------------
// Creates a real context as a specific version (major.minor)
static HGLRC CreateRenderContext( HDC hdc, int major, int minor ) 
{
	// So similar to creating the temp one - we want to define 
	// the style of surface we want to draw to.  But now, to support
	// extensions, it takes key_value pairs
	int const format_attribs[] = {
									WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,		// The rc will be used to draw to a window
									WGL_SUPPORT_OPENGL_ARB, GL_TRUE,		// ...can be drawn to by GL
									WGL_DOUBLE_BUFFER_ARB, GL_TRUE,			// ...is double buffered
									WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,	// ...uses a RGBA texture
									WGL_COLOR_BITS_ARB, 24,					// 24 bits for color (8 bits per channel)
									// WGL_DEPTH_BITS_ARB, 24,				// if you wanted depth a default depth buffer...
									// WGL_STENCIL_BITS_ARB, 8,				// ...you could set these to get a 24/8 Depth/Stencil.
									NULL, NULL,								// Tell it we're done.
								};

	// Given the above criteria, we're going to search for formats
	// our device supports that give us it.  I'm allowing 128 max returns (which is overkill)
	size_t const MAX_PIXEL_FORMATS = 128;
	int formats[MAX_PIXEL_FORMATS];
	int pixel_format = 0;
	UINT format_count = 0;

	BOOL succeeded = wglChoosePixelFormatARB( 
												hdc, 
												format_attribs, 
												nullptr, 
												MAX_PIXEL_FORMATS, 
												formats, 
												(UINT*)&format_count
											);

	if (!succeeded) 
	{
		return NULL; 
	}

	// Loop through returned formats, till we find one that works
	for (unsigned int i = 0; i < format_count; ++i) 
	{
		pixel_format = formats[i];
		succeeded = SetPixelFormat( hdc, pixel_format, NULL ); // same as the temp context; 
		if (succeeded)
		{
			break;
		} else
		{
			DWORD error = GetLastError();
			DebuggerPrintf( "Failed to set the format: %u\n", error ); 
		}
	}

	if (!succeeded) 
	{
		return NULL; 
	}

	// Okay, HDC is setup to the right format, now create our GL context

	// First, options for creating a debug context (potentially slower, but 
	// driver may report more useful errors). 
	int context_flags = 0; 
#if defined(_DEBUG)
	context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB; 
#endif

	// describe the context
	int const attribs[] = {
								WGL_CONTEXT_MAJOR_VERSION_ARB, major,                             // Major GL Version
								WGL_CONTEXT_MINOR_VERSION_ARB, minor,                             // Minor GL Version
								WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,   // Restrict to core (no compatibility)
								WGL_CONTEXT_FLAGS_ARB, context_flags,                             // Misc flags (used for debug above)
								0, 0
							};

	// Try to create context
	HGLRC context = wglCreateContextAttribsARB( hdc, NULL, attribs );
	if (context == NULL) 
	{
		return NULL; 
	}

	return context;
}


//-------------------------------------------------------------------------------------------------------
void BindGLFunctions()
{
	GL_BIND_FUNCTION(glViewport);

	GL_BIND_FUNCTION(glClear);
	GL_BIND_FUNCTION(glClearColor);

	GL_BIND_FUNCTION(glEnable);
	GL_BIND_FUNCTION(glDisable);

	GL_BIND_FUNCTION(glLineWidth);

	GL_BIND_FUNCTION(glBlendFunc);

	GL_BIND_FUNCTION(glCreateShader);
	GL_BIND_FUNCTION(glDeleteShader);
	GL_BIND_FUNCTION(glShaderSource);
	GL_BIND_FUNCTION(glCompileShader);
	GL_BIND_FUNCTION(glGetShaderiv);
	GL_BIND_FUNCTION(glGetShaderInfoLog);
	GL_BIND_FUNCTION(glAttachShader);
	GL_BIND_FUNCTION(glDetachShader);

	GL_BIND_FUNCTION(glCreateProgram);
	GL_BIND_FUNCTION(glDeleteProgram);
	GL_BIND_FUNCTION(glLinkProgram);
	GL_BIND_FUNCTION(glGetProgramiv);
	GL_BIND_FUNCTION(glGetProgramInfoLog);
	GL_BIND_FUNCTION(glUseProgram);

	GL_BIND_FUNCTION(glGenBuffers);
	GL_BIND_FUNCTION(glDeleteBuffers);
	GL_BIND_FUNCTION(glBindBuffer);
	GL_BIND_FUNCTION(glBufferData);

	GL_BIND_FUNCTION(glGenVertexArrays);
	GL_BIND_FUNCTION(glDeleteVertexArrays);
	GL_BIND_FUNCTION(glBindVertexArray);

	GL_BIND_FUNCTION(glGetAttribLocation);
	GL_BIND_FUNCTION(glEnableVertexAttribArray);
	GL_BIND_FUNCTION(glDisableVertexAttribArray);
	GL_BIND_FUNCTION(glVertexAttribPointer);

	GL_BIND_FUNCTION(glDrawArrays);
	GL_BIND_FUNCTION(glDrawElements);

	GL_BIND_FUNCTION(glGetUniformLocation);
	GL_BIND_FUNCTION(glUniformMatrix4fv);
	GL_BIND_FUNCTION(glUniform1i);	
	GL_BIND_FUNCTION(glUniform1ui);
	GL_BIND_FUNCTION(glUniform1f);
	GL_BIND_FUNCTION(glUniform2fv);
	GL_BIND_FUNCTION(glUniform3fv);
	GL_BIND_FUNCTION(glUniform4fv);

	GL_BIND_FUNCTION(glGenSamplers);
	GL_BIND_FUNCTION(glDeleteSamplers);
	GL_BIND_FUNCTION(glSamplerParameteri);
	GL_BIND_FUNCTION(glSamplerParameterfv);
	GL_BIND_FUNCTION(glBindSampler);

	GL_BIND_FUNCTION(glBindTexture);
	GL_BIND_FUNCTION(glActiveTexture);
	GL_BIND_FUNCTION(glPixelStorei);
	GL_BIND_FUNCTION(glGenTextures);
	GL_BIND_FUNCTION(glTexParameteri);
	GL_BIND_FUNCTION(glTexImage2D);
	GL_BIND_FUNCTION(glCopyImageSubData);
	GL_BIND_FUNCTION(glTexStorage2D);
	GL_BIND_FUNCTION(glTexSubImage2D);

	GL_BIND_FUNCTION(glGenerateMipmap);

	GL_BIND_FUNCTION(glGenFramebuffers);
	GL_BIND_FUNCTION(glDeleteFramebuffers);
	GL_BIND_FUNCTION(glBindFramebuffer);
	GL_BIND_FUNCTION(glFramebufferTexture);
	GL_BIND_FUNCTION(glDrawBuffers);
	GL_BIND_FUNCTION(glCheckFramebufferStatus);

	GL_BIND_FUNCTION(glBlitFramebuffer);

	GL_BIND_FUNCTION(glDepthFunc);
	GL_BIND_FUNCTION(glDepthMask);
	GL_BIND_FUNCTION(glClearDepthf);
	
	GL_BIND_FUNCTION(glGetTexImage);

	GL_BIND_FUNCTION(glGetError);

	GL_BIND_FUNCTION(glPolygonMode);

	GL_BIND_FUNCTION(glCullFace);

	GL_BIND_FUNCTION(glFrontFace);

	GL_BIND_FUNCTION(glGetBooleanv);

	GL_BIND_FUNCTION(glBlendEquationSeparate);
	GL_BIND_FUNCTION(glBlendFuncSeparate);

	GL_BIND_FUNCTION(glBindBufferBase);

	GL_BIND_FUNCTION(glGetActiveUniformBlockiv);
	GL_BIND_FUNCTION(glGetActiveUniformBlockName);
	GL_BIND_FUNCTION(glGetActiveUniformsiv);
	GL_BIND_FUNCTION(glGetActiveUniformName	);	
}


//-------------------------------------------------------------------------------------------------------
void LoadShaders()
{
	// Get the csv list of shader files we want to preload
	std::string shaderFilepaths = g_gameConfigBlackboard.GetValue("renderer_ShaderFilesToLoad", "");
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


//-------------------------------------------------------------------------------------------------------
void LoadMaterials()
{
	// Get the csv list of material files we want to preload
	std::string materialFilepaths = g_gameConfigBlackboard.GetValue("renderer_MaterialFilesToLoad", "");
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

//-------------------------------------------------------------------------------------------------------
void Renderer::Initialize()
{
	// Initialize OpenGL --------------------------------------------------------------------------------
	// load the OpenGL dll
	gGLLibrary = ::LoadLibraryA( "opengl32.dll" ); 

	// Get win32 handles
	HWND hwnd = GetActiveWindow();
	HDC hdc = ::GetDC( hwnd );       

	// Create a dummy rendering context so we can get the functions we need to create the real context
	HGLRC dummyContext = CreateDummyRenderContext( hdc ); 
	wglMakeCurrent( hdc, dummyContext ); 
	BindGLContextCreationFunctions(); 

	// create the real context, using OpenGL version 4.2
	HGLRC realContext = CreateRenderContext( hdc, 4, 2 ); 
	wglMakeCurrent( hdc, realContext ); 
	wglDeleteContext( dummyContext ); 

	// Bind all our OpenGL functions we'll be using.
	BindGLFunctions(); 

	// set the globals
	gGLwnd = hwnd;
	gHDC = hdc; 
	gGLContext = realContext; 



	// Initialize Renderer ---------------------------------------------------------------------------------
	// Load Resources
	LoadShaders();
	LoadMaterials();

	// Flags
	EnableCapability(Renderer::CAPABILITY_LINE_SMOOTH);
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Time
	TimeUniformBlock tub;
	tub.m_systemLifetime = g_theMasterClock->GetLifetimeInSeconds();
	tub.m_gameLifetime = g_theGameClock->GetLifetimeInSeconds();
	tub.m_sytstemDeltaSeconds = g_theMasterClock->GetDeltaSeconds();
	tub.m_gameDeltaSeconds = g_theGameClock->GetDeltaSeconds();
	m_timeBlock = UniformBlock();
	m_timeBlock.Set<TimeUniformBlock>(tub);

	// Clear
	m_clearBlock = UniformBlock();
	m_clearBlock.Set<ClearUniformBlock>(ClearUniformBlock());
	m_clearShader.SetShaderProgram(ShaderProgram::CreateOrGet("Data/Shaders/Clear"));

	// Lights
	m_lightBlock = UniformBlock();
	m_lightBlock.Set<LightUniformBlock>(LightUniformBlock());
	SetAmbientLight(RGBA(), 0.0f);
	DisableAllPunctualLights();

	// Model
	m_modelBlock = UniformBlock();
	m_modelBlock.Set<ModelUniformBlock>(ModelUniformBlock());

	// Mesh
	glGenVertexArrays( 1, &m_defaultVAO ); 
	glBindVertexArray( m_defaultVAO ); 

	// RenderTargets
	IntVector2 windowDimensions = g_theWindow->GetDimensions();
	m_defaultColorTarget = CreateRenderTarget( windowDimensions.x, windowDimensions.y );
	m_defaultDepthTarget = CreateRenderTarget( windowDimensions.x, windowDimensions.y, TEXTURE_FORMAT_D24S8 ); 
	m_defaultCamera = new Camera();
	m_defaultCamera->SetColorTarget(m_defaultColorTarget);
	m_defaultCamera->SetDepthStencilTarget(m_defaultDepthTarget);
	m_defaultCamera->Finalize();

	// Screen effects
	m_screenEffectsCamera = new Camera();
	m_screenEffectScratchTarget = CreateRenderTarget( windowDimensions.x, windowDimensions.y );
}


//-------------------------------------------------------------------------------------------------------
void Renderer::Destroy()
{
	// Destroy Renderer ---------------------------------------------------------------------------------
	// Screen effects
	delete m_screenEffectsCamera;
	m_screenEffectsCamera = nullptr;

	// RenderTargets
	delete m_defaultColorTarget;
	m_defaultColorTarget = nullptr;
	
	delete m_defaultDepthTarget;
	m_defaultDepthTarget = nullptr;

	delete m_defaultCamera;
	m_defaultCamera = nullptr;

	// Mesh
	glDeleteVertexArrays(1, &m_defaultVAO);

	// Model
	m_modelBlock.ClearCPUData();

	// Lights
	m_lightBlock.ClearCPUData();

	// Clear
	m_clearBlock.ClearCPUData();
	m_clearShader.SetShaderProgram(nullptr);



	// Destroy OpenGL -----------------------------------------------------------------------------------
	// Release OpenGL context
	wglMakeCurrent( gHDC, NULL ); 
	wglDeleteContext( gGLContext ); 

	// Release win32 handles
	ReleaseDC( gGLwnd, gHDC ); 

	// Clear globals
	gGLContext = NULL; 
	gHDC = NULL;
	gGLwnd = NULL; 

	// Release OpenGL dll
	FreeLibrary( gGLLibrary ); 
}


#include "Engine/Profiler/ProfileScope.hpp"
//-------------------------------------------------------------------------------------------------------
void Renderer::BeginFrame()
{
	// To suppress RenderDoc bug
	for (int i = 0; i < 16; ++i) 
	{
		glDisableVertexAttribArray( (GLint)i );
	}


	// Update Time Block
	TimeUniformBlock* tub = m_timeBlock.As<TimeUniformBlock>();
	tub->m_systemLifetime = g_theMasterClock->GetLifetimeInSeconds();
	tub->m_gameLifetime = g_theGameClock->GetLifetimeInSeconds();
	tub->m_sytstemDeltaSeconds = g_theMasterClock->GetDeltaSeconds();
	tub->m_gameDeltaSeconds = g_theGameClock->GetDeltaSeconds();
	BindUniformBlock(m_timeBlock, TIME_BUFFER_BIND_POINT);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::EndFrame()
{
	TODO("Better Sleep");
	SwapScreenBuffers();
	Sleep(5);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::EnableCapability(unsigned int capability)
{
	glEnable((GLenum)capability);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DisableCapability(unsigned int capability)
{
	glDisable((GLenum)capability);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetViewport(const Vector2& lowerLeft, const Vector2& upperRight)
{
	Vector2 dimensions = upperRight - lowerLeft;
	glViewport((GLint)lowerLeft.x, (GLint)lowerLeft.y, (GLsizei)dimensions.x, (GLsizei)dimensions.y);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindCamera(Camera* camera)
{
	camera->Finalize();
	camera->UpdateUniformBuffer(CAMERA_BUFFER_BIND_POINT);

	SetViewport((Vector2)camera->GetViewportLowerLeft(), (Vector2)camera->GetViewportUpperRight());
}



//-------------------------------------------------------------------------------------------------------
void Renderer::ClearColor(const RGBA& color)
{
	// Set up the render state for a color clear
	RenderState renderState;	   
	renderState.m_depthCamparisonOp = DEPTH_COMPARE_ALWAYS;   
	renderState.m_writeDepth		= false;			
						   
	renderState.m_colorBlendOp		= BLEND_OP_ADD;        
	renderState.m_colorSrcFactor	= BLEND_FACTOR_SOURCE_COLOR;		
	renderState.m_colorDstFactor	= BLEND_FACTOR_ZERO;		
	
	renderState.m_alphaBlendOp		= BLEND_OP_ADD;        
	renderState.m_alphaSrcFactor	= BLEND_FACTOR_SOURCE_ALPHA;		
	renderState.m_alphaDstFactor	= BLEND_FACTOR_ZERO;
	m_clearShader.SetRenderState(renderState);
	
	// Update the uniform block for color clearing
	ClearUniformBlock data;
	data.CLEAR_COLOR		= color.GetAsVector4();
	data.SHOULD_CLEAR_DEPTH = 1.0f;
	data.DEPTH_TO_CLEAR_TO	= 0.0f;
	m_clearBlock.Set<ClearUniformBlock>(data);
	BindUniformBlock(m_clearBlock, CLEAR_BUFFER_BIND_POINT);
	
	// Draw a full screen quad
	const Mesh* fsq = Mesh::GetFSQ();
	BindShader(&m_clearShader);
	DrawMesh(fsq->GetSubMesh(0));
}


//-------------------------------------------------------------------------------------------------------
void Renderer::ClearDepth( float depth)
{
	// Depth writing must be enabled to clear the depth buffer.
	// Check to see is depth buffer can be written to
	GLboolean depthEnabled;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depthEnabled);
	if (depthEnabled == GL_FALSE)
	{
		// If it cant, enable it
		glDepthMask(GL_TRUE);
	}



	// Do the clear
	// Set up the render state for a depth clear
	RenderState renderState;	   
	renderState.m_depthCamparisonOp = DEPTH_COMPARE_ALWAYS;   
	renderState.m_writeDepth		= true;			
	
	renderState.m_colorBlendOp		= BLEND_OP_ADD;        
	renderState.m_colorSrcFactor	= BLEND_FACTOR_ZERO;		
	renderState.m_colorDstFactor	= BLEND_FACTOR_DESTINATION_COLOR;		
	
	renderState.m_alphaBlendOp		= BLEND_OP_ADD;        
	renderState.m_alphaSrcFactor	= BLEND_FACTOR_ZERO;		
	renderState.m_alphaDstFactor	= BLEND_FACTOR_DESTINATION_ALPHA;
	m_clearShader.SetRenderState(renderState);
	
	// Update the uniform block for depth clearing
	ClearUniformBlock data;
	data.CLEAR_COLOR		= Vector4::ONES;
	data.SHOULD_CLEAR_DEPTH = 0.0f;
	data.DEPTH_TO_CLEAR_TO	= depth;
	m_clearBlock.Set<ClearUniformBlock>(data);
	BindUniformBlock(m_clearBlock, CLEAR_BUFFER_BIND_POINT);

	// Draw a full screen quad
	const Mesh* fsq = Mesh::GetFSQ();
	BindShader(&m_clearShader);
	DrawMesh(fsq->GetSubMesh(0));



	// If we started with depth writing disabled
	if (depthEnabled == GL_FALSE)
	{
		// Disable it again
		glDepthMask(GL_FALSE);
	}
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SwapScreenBuffers()
{
	CopyFrameBuffer( nullptr, &(m_defaultCamera->m_output) ); 

	HWND hWnd = GetActiveWindow();
	HDC hDC = GetDC( hWnd );
	SwapBuffers(hDC);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindTextureCube(unsigned int bindPoint, const TextureCube* textureCube)
{
	glActiveTexture(GL_TEXTURE0 + bindPoint);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube->GetHandle());
}


//-------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateRenderTarget( unsigned int width, unsigned int height, unsigned int fmt )
{
	Texture* texture = new Texture();

	texture->CreateRenderTarget( width, height, fmt );    

	return texture;
}


//-------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateDepthStencilTarget( unsigned int width, unsigned int height ) 
{
	return CreateRenderTarget( width, height, TEXTURE_FORMAT_D24S8 ); 
}


//-------------------------------------------------------------------------------------------------------
bool Renderer::CopyFrameBuffer( FrameBuffer *dst, FrameBuffer *src )
{
	// we need at least the src.
	if (src == nullptr) 
	{
		return false; 
	}

	// Get the handles - NULL refers to the "default" or back buffer FBO
	GLuint src_fbo = src->GetHandle();
	GLuint dst_fbo = NULL; 
	if (dst != nullptr) 
	{
		dst_fbo = dst->GetHandle(); 
	}

	// can't copy onto ourselves
	if (dst_fbo == src_fbo) 
	{
		return true; 
	}

	// the GL_READ_FRAMEBUFFER is where we copy from
	glBindFramebuffer( GL_READ_FRAMEBUFFER, src_fbo ); 

	// what are we copying to?
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, dst_fbo ); 

	// blit it over - get the size
	// (we'll assume dst matches for now - but to be safe,
	// you should get dst_width and dst_height using either
	// dst or the window depending if dst was nullptr or not
	IntVector2 srcDimensions = src->GetDimensions();

	// Copy it over
	glBlitFramebuffer(
						0, 0,									// src start pixel
						srcDimensions.x, srcDimensions.y,       // src size
						0, 0,									// dst start pixel
						srcDimensions.x, srcDimensions.y,       // dst size
						GL_COLOR_BUFFER_BIT,					// what are we copying (just color)
						GL_NEAREST								// resize filtering rule (in case src/dst don't match)
					);							


	// cleanup after ourselves
	glBindFramebuffer( GL_READ_FRAMEBUFFER, NULL ); 
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, NULL ); 

	return true;
}


//-------------------------------------------------------------------------------------------------------
Texture* Renderer::GetDefaultColorTarget() const
{
	return m_defaultColorTarget;
}


//-------------------------------------------------------------------------------------------------------
Texture* Renderer::GetDefaultDepthStencilTarget() const
{
	return m_defaultDepthTarget;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindMeshToProgram( const Shader* shader, const SubMesh *mesh ) 
{
	PROFILE_SCOPE_FUNCTION();

	BindVertexBuffer(mesh->m_vertexBuffer);
	if (mesh->m_drawInstruction.IsIndexed())
	{
		BindIndexBuffer(mesh->m_indexBuffer);
	}

	BindVertexLayoutToShaderProgram(mesh->GetVertexLayout(), shader->GetShaderProgram());
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindArrayBuffer(unsigned int target, RenderBuffer& arrayBuffer)
{
	glBindBuffer( (GLenum)target, arrayBuffer.GetArrayBufferHandle() );
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindVertexBuffer(const RenderBuffer& vertexBuffer)
{ 
	glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer.GetArrayBufferHandle() ); 
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindIndexBuffer(const RenderBuffer& indexBuffer)
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer.GetArrayBufferHandle() );
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindVertexLayoutToShaderProgram(const VertexLayout* vertexLayout, const ShaderProgram* shaderProgram)
{
	// For each attribute in the layout
	unsigned int programHandle = shaderProgram->GetProgramHandle();
	int attributeCount = vertexLayout->GetAttributeCount(); 
	for (int i = 0; i < attributeCount; ++i) 
	{
		const VertexAttribute* attribute = vertexLayout->GetAttribute(i); 

		// Get its bind point
		GLint bind = glGetAttribLocation( programHandle, attribute->GetName().c_str() );
		if (bind >= 0)
		{
			// Bind it if the shader uses it
			glEnableVertexAttribArray( bind ); 
			glVertexAttribPointer(  bind, 
									attribute->GetElementCount(), 
									(GLenum)attribute->GetGLType(), 
									attribute->ShouldNormalize(), 
									vertexLayout->GetStride(), 
									(void*)attribute->GetMemberOffset()
								  ); 
		}
	}
}


#include "Engine/Profiler/ProfileScope.hpp"
//-------------------------------------------------------------------------------------------------------
void Renderer::BindUniformBlock(UniformBlock& uniformBuffer, unsigned int bufferBindPoint)
{
	uniformBuffer.BindTo(bufferBindPoint);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindModelMatrix(const Matrix4& modelMatrix)
{
	// Update the buffer
	ModelUniformBlock* modelBlock = m_modelBlock.As<ModelUniformBlock>();
	modelBlock->m_model = modelMatrix;

	// Bind the buffer
	m_modelBlock.BindTo(MODEL_BUFFER_BIND_POINT);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawIndexed(const DrawInstruction& drawInstruction)
{
	PROFILE_SCOPE_FUNCTION();
	glDrawElements( (GLuint)drawInstruction.m_primitiveType, drawInstruction.m_elementCount, GL_UNSIGNED_INT, 0);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawArrays(const DrawInstruction& drawInstruction)
{
	PROFILE_SCOPE_FUNCTION();
	glDrawArrays( (GLuint)drawInstruction.m_primitiveType, 0, drawInstruction.m_elementCount);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindMaterial(const Material* material)
{
	PROFILE_SCOPE_FUNCTION();

	// use the shader program from the material
	// set the render state from the shaders renderstate
	const Shader* shader = material->GetShader();
	BindShader(shader);


	// bind the textures from the material
	for (int i = 0; i < (int)material->GetNumberOfTextures(); ++i)
	{
		if (material->IsValidTexture(i))
		{
			BindTextureAndSampler(i, material->GetTexture(i), material->GetSampler(i));
		}
	}


	// bind any material properties 
	material->BindPropertyBuffers();
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindShader(const Shader* shader)
{
	m_currentShader = shader;
	BindShaderProgram(shader->GetShaderProgram()); 
	BindRenderState(shader->GetRenderState());
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindShaderProgram(const ShaderProgram* shaderProgram)
{
	glUseProgram( shaderProgram->GetProgramHandle() ); 
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindRenderState(const RenderState& renderState)
{
	// blend mode
	glEnable( GL_BLEND ); 
	glBlendEquationSeparate(renderState.m_colorBlendOp, renderState.m_alphaBlendOp);
	glBlendFuncSeparate(renderState.m_colorSrcFactor, renderState.m_colorDstFactor, renderState.m_alphaSrcFactor, renderState.m_alphaDstFactor);
	
	// Depth mode ones
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(renderState.m_depthCamparisonOp); 
	glDepthMask(renderState.m_writeDepth);

	// Fill mode
	glPolygonMode(GL_FRONT_AND_BACK, renderState.m_fillMode);

	// Cull mode
	if (renderState.m_cullMode == CULL_NONE)
	{
		glDisable(Renderer::CAPABILITY_BACKFACE_CULLING);
	}
	else
	{
		glEnable(Renderer::CAPABILITY_BACKFACE_CULLING);
		glCullFace((GLenum)renderState.m_cullMode);
	}

	// Winding Order
	glFrontFace((GLenum)renderState.m_windingOrder);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindTextureAndSampler(unsigned int bindPoint, const Texture* texture, const Sampler* sampler)
{
	BindTexture(bindPoint, texture);
	BindSampler(bindPoint, sampler);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindTexture(unsigned int bindPoint, const Texture* texture)
{
	glActiveTexture( GL_TEXTURE0 + bindPoint ); 
	glBindTexture( GL_TEXTURE_2D, (GLuint)texture->GetHandle() );
}


//-------------------------------------------------------------------------------------------------------
void Renderer::BindSampler(unsigned int bindPoint, const Sampler* sampler)
{
	glBindSampler( bindPoint, sampler->GetSamplerHandle() ); 
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawMesh(const SubMesh* mesh)
{
	PROFILE_SCOPE_FUNCTION();

	BindMeshToProgram(m_currentShader, mesh);



	// JIT UNIFORMS
	m_lightBlock.BindTo(LIGHT_BUFFER_BIND_POINT);


	// Draw
	if (mesh->m_drawInstruction.IsIndexed())
	{
		DrawIndexed(mesh->m_drawInstruction);
	}
	else
	{
		DrawArrays(mesh->m_drawInstruction);
	}
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawMeshImmediate( const Vertex_3DPCU* verts, int numVerts, const unsigned int* indices, int numIndices, unsigned int drawPrimitive)
{
	SubMesh mesh;

	mesh.m_drawInstruction.m_startIndex		= 0;
	mesh.m_drawInstruction.m_primitiveType	= drawPrimitive;
	mesh.m_drawInstruction.m_usingIndices	= false;
	mesh.m_drawInstruction.m_elementCount	= 0;

	mesh.SetVertices(numVerts, verts); 
	if (numIndices != 0 && indices != nullptr)
	{
		mesh.SetIndices(numIndices, indices);
	}
	mesh.SetVertexLayout(&(Vertex_3DPCU::s_layout));

	DrawMesh(&mesh);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawSprite( const Sprite* sprite, const RGBA& tint, const Vector3& position, const Vector3& right, const Vector3& up, const Vector2& scale)
{
	TODO("Actually use scale for scale not just flipping sprites");

	// 3 - 2
	// | / |
	// 0 - 1
	Vector2 spriteWorldDim = sprite->GetWorldDimensions();
	Vector2 spritePivot = sprite->GetPivot();
	AABB2D  spriteUVs = sprite->GetUVCoordinates();

	// Swap UVs if we have a negative scale
	// Flip sprite around pivot
	if (scale.x < 0.0f)
	{
		float temp = spriteUVs.mins.x;
		spriteUVs.mins.x = spriteUVs.maxs.x;
		spriteUVs.maxs.x = temp;
		spritePivot.x = 1.0f - spritePivot.x;
	}
	if (scale.y < 0.0f)
	{
		float temp = spriteUVs.mins.y;
		spriteUVs.mins.y = spriteUVs.maxs.y;
		spriteUVs.maxs.y = temp;
		spritePivot.y = 1.0f - spritePivot.y;
	}

	Vector3 cubePoints[4];
	cubePoints[0] = position + (right * (spriteWorldDim.x * -(spritePivot.x))) + (up * (spriteWorldDim.y * -(spritePivot.y)));
	cubePoints[1] = cubePoints[0] + (right * spriteWorldDim.x);
	cubePoints[2] = cubePoints[0] + (right * spriteWorldDim.x) + (up * spriteWorldDim.y);
	cubePoints[3] = cubePoints[0] + (up * spriteWorldDim.y);

	Vertex_3DPCU vertices[6];

	// 012 023
	vertices[0] = Vertex_3DPCU(cubePoints[0], tint, Vector2(spriteUVs.mins.x, spriteUVs.mins.y));
	vertices[1] = Vertex_3DPCU(cubePoints[1], tint, Vector2(spriteUVs.maxs.x, spriteUVs.mins.y));
	vertices[2] = Vertex_3DPCU(cubePoints[2], tint, Vector2(spriteUVs.maxs.x, spriteUVs.maxs.y));

	vertices[3] = Vertex_3DPCU(cubePoints[0], tint, Vector2(spriteUVs.mins.x, spriteUVs.mins.y));
	vertices[4] = Vertex_3DPCU(cubePoints[2], tint, Vector2(spriteUVs.maxs.x, spriteUVs.maxs.y));
	vertices[5] = Vertex_3DPCU(cubePoints[3], tint, Vector2(spriteUVs.mins.x, spriteUVs.maxs.y));

	BindTexture(DIFFUSE_TEXTURE_BIND_POINT, sprite->GetTexture());
	//BindDiffuseTexture(sprite->GetTexture());
	DrawMeshImmediate(vertices, 6, nullptr, 0, PRIMITIVE_TRIANGLES);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawIsoSprite( const IsoSprite* isoSprite, const RGBA& tint, const Vector3& spritePosition, const Vector3& spriteForward, const Vector3& spriteRight, const Vector3& spriteUp,
								const Vector3& viewerForward, const Vector3& viewerRight, const Vector2& scale)
{
	UNUSED(scale);

	Vector2 isoScale = Vector2(1.0f, 1.0f);
	const Sprite* sprite = isoSprite->GetSpriteForView(spriteForward, spriteRight, viewerForward, &isoScale);
	DrawSprite(sprite, tint, spritePosition, viewerRight, spriteUp, isoScale);
}


//-------------------------------------------------------------------------------------------------------
#include "Engine/Rendering/MeshBuilder.hpp"
void Renderer::DrawText2D( const Vector2& drawMins,
						   const std::string& asciiText,
						   float cellHeight,
						   RGBA  tint = RGBA(255, 255, 255),
						   float aspectScale = 1.f, // multiplied by the font’s inherent m_baseAspect
						   const BitmapFont* font = nullptr,
						   float percentageComplete)
{
	// No default font yet so if one is not provided just dont draw for now.
	if (font == nullptr)
	{
		return;
	}
	//SetDiffuseTexture(font->GetTexture());
	BindShader(Shader::Get("Default_2D"));
	BindTextureAndSampler(DIFFUSE_TEXTURE_BIND_POINT, font->GetTexture(), Sampler::GetPointSampler());

	MeshBuilder builder;
	builder.Initialize(PRIMITIVE_TRIANGLES, false);

	AddTextToMeshBuilder(builder, drawMins, tint, font, cellHeight, aspectScale, asciiText, percentageComplete);

	builder.Finalize();
	Mesh* mesh = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	DrawMesh(mesh->GetSubMesh(0));
	delete mesh;
	return;
}

#include <list>
char * StringFindLastSpaceFromPosition(char* string, char* position = nullptr)
{
	char* lastSpace = nullptr;

	if (position == nullptr)
	{
		lastSpace = strrchr(string, ' ');
	}
	else
	{
		while(position != string)
		{
			--position;
			if (*position == ' ')
			{
				lastSpace = position;
				break;
			}
		}
	}

	return lastSpace;
}

void Renderer::WordWrapLines(std::list<char*>& lines, float boxWidth, float cellHeight, float aspectScale, const BitmapFont* font)
{
	for (auto lineIter = lines.begin(); lineIter != lines.end(); ++lineIter)
	{
		float lineWidth = font->GetStringWidth(*lineIter, cellHeight, aspectScale);
		if (lineWidth > boxWidth)
		{
			char* lastSpace = nullptr;
			while (font->GetStringWidth(*lineIter, cellHeight, aspectScale) > boxWidth)
			{
				lastSpace = StringFindLastSpaceFromPosition(*lineIter, lastSpace);

				*lastSpace = '\0';
				if (font->GetStringWidth(*lineIter, cellHeight, aspectScale) < boxWidth)
				{
					++lastSpace;
					++lineIter;
					lines.insert(lineIter, lastSpace);
					--lineIter;
					--lineIter;
				}
				else
				{
					*lastSpace = ' ';
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------------
void Renderer::DrawTextInBox2D(const AABB2D& bounds, const std::string& asciiText, float cellHeight, RGBA tint,
							   float aspectScale, const BitmapFont* font, const Vector2& alignment, TextDrawMode drawMode, float percentageComplete)
{
	char* editableText = CopyString(asciiText);
	std::list<char*> lines = TokenizeString(editableText, "\n");

	Vector2 boxDimensions = bounds.GetDimensions();
	float scale = 1.0f;
	if (drawMode == TEXT_DRAW_WORD_WRAP)
	{
		WordWrapLines(lines, boxDimensions.x, cellHeight, aspectScale, font);
	}
	else if (drawMode == TEXT_DRAW_SHRINK_TO_FIT)
	{
		float heightScale = 1.0f;
		float widthScale  = 1.0f;

		float textHeight = cellHeight * lines.size();
		if (textHeight > boxDimensions.y)
		{
			heightScale = boxDimensions.y / textHeight;
		}

		float longestLineWidth = 0.0f;
		for (auto lineIter = lines.begin(); lineIter != lines.end(); ++lineIter)
		{
			float lineWidth = font->GetStringWidth(*lineIter, cellHeight, aspectScale);
			if (lineWidth > longestLineWidth)
			{
				longestLineWidth = lineWidth;
			}
		}
		if (longestLineWidth > boxDimensions.x)
		{
			widthScale = boxDimensions.x / longestLineWidth;
		}
		scale = Min(heightScale, widthScale);
	}


	float textHeight = (cellHeight * scale) * lines.size();
	float totalVerticalPadding = boxDimensions.y - textHeight;
	Vector2 offset = Vector2(0.0f, totalVerticalPadding * alignment.y);
	for(auto lineReverseIter = lines.rbegin(); lineReverseIter != lines.rend(); lineReverseIter++)
	{
		float textWidth = font->GetStringWidth(*lineReverseIter, cellHeight * scale, aspectScale);
		offset.x = (boxDimensions.x - textWidth) * alignment.x;

		DrawText2D(bounds.mins + offset, *lineReverseIter, cellHeight * scale, tint, aspectScale, font, percentageComplete);
		offset.y += (cellHeight * scale);
	}
	delete[] editableText;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawText3D(const Vector3& position, const Vector3& right, const Vector3& up,
								const std::string& asciiText, float cellHeight, RGBA tint, float aspectScale, 
								const BitmapFont* font, float percentageComplete)
{
	// No default font yet so if one is not provided just dont draw for now.
	// Or if there was no text provided
	if (font == nullptr || 
		asciiText.length() == 0)
	{
		return;
	}
	const Texture* fontTexture = font->GetTexture();

	float percentagePerCharacter = 1.0f;
	if (asciiText.size() > 0) { percentagePerCharacter = 1.0f / (float)asciiText.size(); }
	float numFadedCharacters = percentageComplete / percentagePerCharacter;
	int numFullyFadedInCharacters = (int)(numFadedCharacters);

	float glyphHeight = cellHeight;
	float glyphWidth  = glyphHeight * (font->GetGlyphAspect(asciiText[0]) * aspectScale);

	Vector3 textMins = position;
	for (size_t charIndex = 0; charIndex < asciiText.length(); ++charIndex)
	{
		if ((int)charIndex == numFullyFadedInCharacters)
		{
			tint.a = Interpolate((unsigned char)0, (unsigned char)255, numFadedCharacters - numFullyFadedInCharacters);
		}
		else if ((int)charIndex > numFullyFadedInCharacters)
		{
			tint.a = 0;
		}


		AABB2D glyphUVs = font->GetUVsForGlyph(asciiText[charIndex]);
		Sprite glyphSprite = Sprite(fontTexture, glyphUVs, Vector2(0.0f, 0.0f), Vector2(glyphWidth, glyphHeight));
		DrawSprite(&glyphSprite, tint, textMins, right, up);

		textMins += ((glyphWidth) * right);
	}
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DrawTextInBox3D(const Vector3& position, const Vector2& dimensions, const Vector2& pivot, const Vector3& right, const Vector3& up, const std::string& asciiText, float cellHeight, RGBA tint, float aspectScale, const BitmapFont* font, const Vector2& alignment, TextDrawMode drawMode, float percentageComplete)
{
	Vector3 truePosition = position + (right * (dimensions.x * -(pivot.x))) + (up * (dimensions.y * -(pivot.y)));

	char* editableText = CopyString(asciiText);
	std::list<char*> lines = TokenizeString(editableText, "\n");

	Vector2 boxDimensions = dimensions;
	float scale = 1.0f;
	if (drawMode == TEXT_DRAW_WORD_WRAP)
	{
		WordWrapLines(lines, boxDimensions.x, cellHeight, aspectScale, font);
	}
	else if (drawMode == TEXT_DRAW_SHRINK_TO_FIT)
	{
		float heightScale = 1.0f;
		float widthScale  = 1.0f;

		float textHeight = cellHeight * lines.size();
		if (textHeight > boxDimensions.y)
		{
			heightScale = boxDimensions.y / textHeight;
		}

		float longestLineWidth = 0.0f;
		for (auto lineIter = lines.begin(); lineIter != lines.end(); ++lineIter)
		{
			float lineWidth = font->GetStringWidth(*lineIter, cellHeight, aspectScale);
			if (lineWidth > longestLineWidth)
			{
				longestLineWidth = lineWidth;
			}
		}
		if (longestLineWidth > boxDimensions.x)
		{
			widthScale = boxDimensions.x / longestLineWidth;
		}
		scale = Min(heightScale, widthScale);
	}


	float textHeight = (cellHeight * scale) * lines.size();
	float totalVerticalPadding = boxDimensions.y - textHeight;
	Vector2 offset = Vector2(0.0f, totalVerticalPadding * alignment.y);
	for(auto lineReverseIter = lines.rbegin(); lineReverseIter != lines.rend(); lineReverseIter++)
	{
		float textWidth = font->GetStringWidth(*lineReverseIter, cellHeight * scale, aspectScale);
		offset.x = (boxDimensions.x - textWidth) * alignment.x;

		// drop shadow like effect
		DrawText3D(truePosition + (offset.y * up) + (right * .03f) + (up * -0.03f), right, up, *lineReverseIter, cellHeight * scale, RGBA(64,64,64, tint.a), aspectScale, font, percentageComplete);
		
		// 4 offset method
		//DrawText3D(truePosition + (offset.y * up) + (right * .02f) + (up * 0.02f), right, up, *lineReverseIter, cellHeight * scale, RGBA(0,0,0), aspectScale, font, percentageComplete);
		//DrawText3D(truePosition + (offset.y * up) + (right * -.025f) + (up * 0.02f), right, up, *lineReverseIter, cellHeight * scale, RGBA(0,0,0), aspectScale, font, percentageComplete);
		//DrawText3D(truePosition + (offset.y * up) + (right * .02f) + (up * -0.02f), right, up, *lineReverseIter, cellHeight * scale, RGBA(0,0,0), aspectScale, font, percentageComplete);
		//DrawText3D(truePosition + (offset.y * up) + (right * -.025f) + (up * -0.02f), right, up, *lineReverseIter, cellHeight * scale, RGBA(0,0,0), aspectScale, font, percentageComplete);

		DrawText3D(truePosition + (offset.y * up), right, up, *lineReverseIter, cellHeight * scale, tint, aspectScale, font, percentageComplete);
		offset.y += (cellHeight * scale);
	}
	delete[] editableText;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::ApplyEffect(const Material* material) 
{
	TODO("Effects on the viewport cameras");

	if (m_screenEffectTarget == nullptr) 
	{
		m_screenEffectTarget = m_defaultColorTarget; 
	}

	// Bind the scratch buffer to our camera
	m_screenEffectsCamera->SetColorTarget( m_screenEffectScratchTarget ); 
	BindCamera( m_screenEffectsCamera ); 

	// Bind the actual effect
	BindMaterial(material);
	
	// Bind our current color target 
	BindTexture(0, m_screenEffectTarget);
	
	const Mesh* fsq = Mesh::GetFSQ();
	DrawMesh(fsq->GetSubMesh(0));

	// Swap effect targets so we can optionally apply more effects
	Texture* tmp = m_screenEffectScratchTarget;
	m_screenEffectScratchTarget = m_screenEffectTarget;
	m_screenEffectTarget = tmp;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::FinishEffects()
{
	// After this call finishes - m_default_color_target should be m_effect_target	
	if (m_screenEffectTarget == nullptr)
	{
		return; 
	}

	if (m_screenEffectTarget != m_defaultColorTarget) 
	{
		// We ended on an odd apply effect
		m_defaultColorTarget->CopyFromTexture( m_screenEffectTarget );
		m_screenEffectScratchTarget = m_screenEffectTarget; 
	}

	// signifying done with effects
	m_screenEffectTarget = nullptr;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetShadowCascadeBreakpoints(const Camera* camera)
{
	// Set the current cascades
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	std::vector<float> cascadeBreakpoints = camera->GetCascadeBreakpoints();

	for (int i = 0; i < NUM_CASCADES && i < (int)cascadeBreakpoints.size() ; ++i)
	{
		block->m_cascadeBreakpoints[i].m_breakpoint = cascadeBreakpoints[i];
	}
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetSunLight(const Light* light, bool setShadowMap)
{
	GUARANTEE_OR_DIE(light->GetLightType() == LIGHT_TYPE_SUN, "SetSunLight should not be passed a light type other than LIGHT_TYPE_SUN");

	// Sun Path
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();

	// Set the actual light values for the 
	block->m_sun = light->GetLightData();

	// Set up the cascade shadow maps
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		ShadowMapData data;
		data.m_viewProjection = light->GetViewProjectionMatrix(i);
		block->m_sunCascades[i] = data;

		const Texture* shadowMap;
		if (setShadowMap)
		{
			shadowMap = light->GetShadowMapForDrawing(i);
		}
		else
		{
			shadowMap = light->GetDefaultShadowMap(); 
		}
		BindTextureAndSampler(SUN_CASCADE_BIND_POINT_0 + i, shadowMap, Light::GetShadowMapSampler());
	}
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DisableSunLight()
{
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	block->m_sun.Disable();
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetAmbientLight(const RGBA& color, float brightness)
{
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	Vector4 ambient = color.GetAsVector4();
	ambient.w = brightness;
	block->m_ambient = ambient;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::DisableAllPunctualLights()
{
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		block->m_lights[i].Disable();
		block->m_shadowMaps->m_viewProjection = Matrix4();
	}
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetShadowMapData(unsigned int index, const Matrix4& viewProjection)
{
	ShadowMapData data;
	data.m_viewProjection = viewProjection;

	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	block->m_shadowMaps[index] = data;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetPunctualLight(unsigned int index, const Light* light, bool setShadowMap)
{
	switch(light->GetLightType())
	{
	case LIGHT_TYPE_POINT:
	case LIGHT_TYPE_CONE:
	case LIGHT_TYPE_DIRECTIONAL:
	{
		// Punctual Path
		SetLightData(index, &(light->GetLightData()));
		SetShadowMapData(index, light->GetViewProjectionMatrix());

		const Texture* shadowMap;
		if (setShadowMap)
		{
			shadowMap = light->GetShadowMapForDrawing();
		}
		else
		{
			shadowMap = light->GetDefaultShadowMap();
		}
		BindTextureAndSampler(SHADOW_MAP_BIND_POINT_0 + index, shadowMap, Light::GetShadowMapSampler());

		break;
	}

	default:
		break;
	}
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetLightData(unsigned int index, const LightData* light)
{
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	block->m_lights[index] = *light;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetPointLightData(unsigned int index, const Vector3& position, const RGBA& color, float intensity, const Vector3& attenuation)
{
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	block->m_lights[index].InitializePointLight(position, color, intensity, attenuation);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetDirectionalLightData(unsigned int index, const Vector3& position, const Vector3& direction, const RGBA& color, float intensity, const Vector3& attenuation)
{
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	block->m_lights[index].InitializeDirectionalLight(position, direction, color, intensity, attenuation);
}


//-------------------------------------------------------------------------------------------------------
void Renderer::SetConeLightData(unsigned int index, const Vector3& position, const Vector3& direction, float innerAngle, float outerAngle, const RGBA& color, float intensity, const Vector3& attenuation)
{
	LightUniformBlock* block = m_lightBlock.As<LightUniformBlock>();
	block->m_lights[index].InitializeConeLight(position, direction, innerAngle, outerAngle, color, intensity, attenuation);
}


//-------------------------------------------------------------------------------------------------------
unsigned int Renderer::StringToPrimitiveType(const std::string& name, unsigned int def)
{
	unsigned int primitive = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "triangles")
	{
		primitive = PRIMITIVE_TRIANGLES;
	}
	else if (lowerName == "lines")
	{
		primitive = PRIMITIVE_LINES;
	}

	return primitive;
}


//-------------------------------------------------------------------------------------------------------
unsigned int Renderer::StringToFillMode(const std::string& name, unsigned int def)
{
	unsigned int fillMode = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "solid")
	{
		fillMode = FILL_SOLID;
	}
	else if (lowerName == "wireframe")
	{
		fillMode = FILL_WIREFRAME;
	}

	return fillMode;
}


//-------------------------------------------------------------------------------------------------------
unsigned int Renderer::StringToCullMode(const std::string& name, unsigned int def)
{
	unsigned int cullMode = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "front" || lowerName == "front face")
	{
		cullMode = CULL_FRONT;
	}
	else if (lowerName == "back" || lowerName == "back face")
	{
		cullMode = CULL_BACK;
	}
	else if (lowerName == "none")
	{
		cullMode = CULL_NONE;
	}

	return cullMode;
}


//-------------------------------------------------------------------------------------------------------
unsigned int Renderer::StringToWindingOrder(const std::string& name, unsigned int def)
{
	unsigned int windingOrder = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "ccw" || lowerName == "counter clockwise")
	{
		windingOrder = WIND_CCW;
	}
	else if (lowerName == "cw" || lowerName == "clockwise")
	{
		windingOrder = WIND_CW;
	}


	return windingOrder;
}


//-------------------------------------------------------------------------------------------------------
unsigned int Renderer::StringToBlendOp(const std::string& name, unsigned int def)
{
	unsigned int op = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "add")
	{
		op = BLEND_OP_ADD;
	}
	else if (lowerName == "subtract" || lowerName == "sub")
	{
		op = BLEND_OP_SUBTRACT;
	}
	else if (lowerName == "revsub" || lowerName == "reverse subtract")
	{
		op = BLEND_OP_REVSUB;
	}
	else if (lowerName == "min")
	{
		op = BLEND_OP_MIN;
	}
	else if (lowerName == "max")
	{
		op = BLEND_OP_MAX;
	}

	return op;
}


//-------------------------------------------------------------------------------------------------------
unsigned int Renderer::StringToBlendFactor(const std::string& name, unsigned int def)
{
	unsigned int blendFactor = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "zero")
	{
		blendFactor = BLEND_FACTOR_ZERO;
	}
	else if (lowerName == "one")
	{
		blendFactor = BLEND_FACTOR_ONE;
	}
	else if (lowerName == "source color" || lowerName == "source_color" || lowerName == "src_color" || lowerName == "src color")
	{
		blendFactor = BLEND_FACTOR_SOURCE_COLOR;
	}
	else if (lowerName == "inverse source color" || lowerName == "inverse_source_color" || lowerName == "inv src color" || lowerName == "inv_src_color")
	{
		blendFactor = BLEND_FACTOR_ONE_MINUS_SOURCE_COLOR;
	}
	else if (lowerName == "source alpha" || lowerName == "source_alpha" || lowerName == "src_alpha" || lowerName == "src alpha")
	{
		blendFactor = BLEND_FACTOR_SOURCE_ALPHA;
	}
	else if (lowerName == "inv_src_alpha" || lowerName == "inv src alpha" || lowerName == "inverse source alpha" || lowerName == "inverse_source_alpha")
	{
		blendFactor = Renderer::BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA;
	}
	else if (lowerName == "dest_color" || lowerName == "dest color")
	{
		blendFactor = Renderer::BLEND_FACTOR_DESTINATION_COLOR;
	}
	else if (lowerName == "inv_dest_color" || lowerName == "inv dest color")
	{
		blendFactor = Renderer::BLEND_FACTOR_ONE_MINUS_DESTINATION_COLOR;
	}
	else if (lowerName == "dest_alpha" || lowerName == "dest alpha")
	{
		blendFactor = Renderer::BLEND_FACTOR_DESTINATION_ALPHA;
	}
	else if (lowerName == "inv_dest_alpha" || lowerName == "inv dest alpha")
	{
		blendFactor = Renderer::BLEND_FACTOR_ONE_MINUS_DESTINATION_ALPHA;
	}

	return blendFactor;
}


//-------------------------------------------------------------------------------------------------------
unsigned int Renderer::StringToDepthTest(const std::string& name, unsigned int def)
{
	unsigned int depthTest = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "less")
	{
		depthTest = DEPTH_COMPARE_LESS;
	}
	else if (lowerName == "never")
	{
		depthTest = DEPTH_COMPARE_NEVER;
	}
	else if (lowerName == "equal")
	{
		depthTest = DEPTH_COMPARE_EQUAL;
	}
	else if (lowerName == "lequal")
	{
		depthTest = DEPTH_COMPARE_LEQUAL;
	}
	else if (lowerName == "greater")
	{
		depthTest = DEPTH_COMPARE_GREATER;
	}
	else if (lowerName == "gequal")
	{
		depthTest = DEPTH_COMPARE_GEQUAL;
	}
	else if (lowerName == "not")
	{
		depthTest = DEPTH_COMPARE_NOT_EQUAL;
	}
	else if (lowerName == "always")
	{
		depthTest = DEPTH_COMPARE_ALWAYS;
	}

	return depthTest;
}


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::PRIMITIVE_LINES		  = GL_LINES;
const unsigned int Renderer::PRIMITIVE_TRIANGLES	  = GL_TRIANGLES;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::CAPABILITY_BACKFACE_CULLING = GL_CULL_FACE;
const unsigned int Renderer::CAPABILITY_BLEND			 = GL_BLEND;
const unsigned int Renderer::CAPABILITY_LINE_SMOOTH		 = GL_LINE_SMOOTH;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::FILL_SOLID					  = GL_FILL;
const unsigned int Renderer::FILL_WIREFRAME				  = GL_LINE;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::CULL_FRONT					  = GL_FRONT;
const unsigned int Renderer::CULL_BACK					  = GL_BACK;
const unsigned int Renderer::CULL_NONE					  = GL_NONE;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::WIND_CW					  = GL_CW;
const unsigned int Renderer::WIND_CCW					  = GL_CCW;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::BLEND_OP_ADD				  = GL_FUNC_ADD;
const unsigned int Renderer::BLEND_OP_SUBTRACT			  = GL_FUNC_SUBTRACT;
const unsigned int Renderer::BLEND_OP_REVSUB			  = GL_FUNC_REVERSE_SUBTRACT;
const unsigned int Renderer::BLEND_OP_MIN				  = GL_MIN;
const unsigned int Renderer::BLEND_OP_MAX				  = GL_MAX;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::BLEND_FACTOR_ZERO							= GL_ZERO;
const unsigned int Renderer::BLEND_FACTOR_ONE							= GL_ONE;
const unsigned int Renderer::BLEND_FACTOR_SOURCE_COLOR					= GL_SRC_COLOR;
const unsigned int Renderer::BLEND_FACTOR_DESTINATION_COLOR				= GL_DST_COLOR;
const unsigned int Renderer::BLEND_FACTOR_ONE_MINUS_SOURCE_COLOR		= GL_ONE_MINUS_SRC_COLOR;
const unsigned int Renderer::BLEND_FACTOR_ONE_MINUS_DESTINATION_COLOR	= GL_ONE_MINUS_DST_COLOR;
const unsigned int Renderer::BLEND_FACTOR_SOURCE_ALPHA					= GL_SRC_ALPHA;
const unsigned int Renderer::BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA		= GL_ONE_MINUS_SRC_ALPHA;
const unsigned int Renderer::BLEND_FACTOR_DESTINATION_ALPHA				= GL_DST_ALPHA;
const unsigned int Renderer::BLEND_FACTOR_ONE_MINUS_DESTINATION_ALPHA	= GL_ONE_MINUS_DST_ALPHA;
const unsigned int Renderer::BLEND_FACTOR_SOURCE_ALPHA_SATURATE			= GL_SRC_ALPHA_SATURATE;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::TEXTURE_2D = GL_TEXTURE_2D;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::TEXTURE_FORMAT_RGBA8 = GL_RGBA8;
const unsigned int Renderer::TEXTURE_FORMAT_D24S8 = GL_DEPTH_STENCIL; 


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::DEPTH_COMPARE_NEVER		= GL_NEVER;    
const unsigned int Renderer::DEPTH_COMPARE_LESS			= GL_LESS;        
const unsigned int Renderer::DEPTH_COMPARE_LEQUAL		= GL_LEQUAL;      
const unsigned int Renderer::DEPTH_COMPARE_GREATER		= GL_GREATER;     
const unsigned int Renderer::DEPTH_COMPARE_GEQUAL		= GL_GEQUAL;      
const unsigned int Renderer::DEPTH_COMPARE_EQUAL		= GL_EQUAL;       
const unsigned int Renderer::DEPTH_COMPARE_NOT_EQUAL	= GL_NOTEQUAL;   
const unsigned int Renderer::DEPTH_COMPARE_ALWAYS		= GL_ALWAYS; 


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::TIME_BUFFER_BIND_POINT					= 1;
const unsigned int Renderer::CAMERA_BUFFER_BIND_POINT				= 2;
const unsigned int Renderer::MODEL_BUFFER_BIND_POINT				= 3;
const unsigned int Renderer::LIGHT_BUFFER_BIND_POINT				= 4;
const unsigned int Renderer::CLEAR_BUFFER_BIND_POINT				= 5;
const unsigned int Renderer::FOG_BUFFER_BIND_POINT					= 6;
const unsigned int Renderer::MATERIAL_PROPERTY_BUFFER_0_BIND_POINT	= 8;


//-------------------------------------------------------------------------------------------------------
const unsigned int Renderer::DIFFUSE_TEXTURE_BIND_POINT		= 0;
const unsigned int Renderer::NORMAL_MAP_BIND_POINT			= 1;
const unsigned int Renderer::SHADOW_MAP_BIND_POINT_0		= 2;
const unsigned int Renderer::SKYBOX_BIND_POINT				= 8;
const unsigned int Renderer::SUN_CASCADE_BIND_POINT_0	    = 10;
