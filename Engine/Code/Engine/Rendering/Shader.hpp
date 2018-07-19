#pragma once

#include <string>
#include <map>

#include "Engine/Core/XmlUtilities.hpp"

class ShaderProgram;



enum RenderQueue
{
	RENDER_QUEUE_INVALID = -1,

	RENDER_QUEUE_OPAQUE = 0,
	RENDER_QUEUE_ALPHA,
	RENDER_QUEUE_PARTICLES,

	RENDER_QUEUE_COUNT
};
RenderQueue StringToRenderQueue(const std::string& name, RenderQueue def = RENDER_QUEUE_OPAQUE);



struct RenderState 
{
	RenderState();

	// Raster State Control
	unsigned int  m_cullMode;			   // CULL_BACK
	unsigned int  m_fillMode;			   // FILL_SOLID
	unsigned int  m_windingOrder;		   // WIND_COUNTER_CLOCKWISE

	// Depth State Control
	unsigned int  m_depthCamparisonOp;     // COMPARE_LESS
	bool		  m_writeDepth;			   // true

	// Blend
	unsigned int  m_colorBlendOp;          // COMPARE_ADD
	unsigned int  m_colorSrcFactor;		   // BLEND_ONE
	unsigned int  m_colorDstFactor;		   // BLEND_ZERO

	unsigned int  m_alphaBlendOp;          // COMPARE_ADD
	unsigned int  m_alphaSrcFactor;		   // BLEND_ONE
	unsigned int  m_alphaDstFactor;		   // BLEND_ONE
}; 



class Shader
{
public:
	Shader();
	Shader(const XMLElement& definition);
	Shader(const Shader& toCopy);
	~Shader();

	unsigned int		 GetRenderLayer() const;
	RenderQueue			 GetRenderQueue() const;
	const ShaderProgram* GetShaderProgram() const;
	const RenderState&   GetRenderState() const;

	void SetRenderLayer(unsigned int layer);
	void SetRenderQueue(RenderQueue queue);
	void SetRenderState(const RenderState& newRenderState);

	// none of these call GL calls, just sets the internal state
	void SetShaderProgram(const ShaderProgram* program); 

	void EnableBlending(unsigned int op, unsigned int src, unsigned int dst); 
	void DisableBlending();

	void SetDepth(unsigned int op, bool shouldWrite); 
	void DisableDepth();

	void SetCullMode(unsigned int cullMode); 
	void SetFillMode(unsigned int fillMode); 
	void SetWindingOrderForFrontFace(unsigned int windOrder); 

	bool IsLit() const;


	static Shader*	Get(const std::string& name);
	static Shader*  Clone(const std::string& nameToClone, const std::string& name = ""); // If a name is specified the shader will be registered
	static void		ParseShaders(const std::string& shaderFile);

private:
	// Bucket by layer then sub bucket by queue
	unsigned int			m_renderLayer; 
	RenderQueue				m_renderQueue;
	const ShaderProgram*	m_shaderProgram; 
	RenderState				m_renderState;

	static std::map<std::string, Shader*> s_loadedShaders;
}; 
