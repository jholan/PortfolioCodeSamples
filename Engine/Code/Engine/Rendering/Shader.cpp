#include "Engine/Rendering/Shader.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/StringUtils.hpp"

#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/Renderer.hpp"

extern Renderer* g_theRenderer;



RenderQueue StringToRenderQueue(const std::string& name, RenderQueue def)
{
	RenderQueue queue = def;

	std::string lowerName = ToLower(name);
	if (lowerName == "opaque")
	{
		queue = RENDER_QUEUE_OPAQUE;
	}
	else if (lowerName == "alpha")
	{
		queue = RENDER_QUEUE_ALPHA;
	}
	else if (lowerName == "particles")
	{
		queue = RENDER_QUEUE_PARTICLES;
	}

	return queue;
}



RenderState::RenderState()
	: m_cullMode(Renderer::CULL_BACK)
	, m_fillMode(Renderer::FILL_SOLID)
	, m_windingOrder(Renderer::WIND_CCW)
	, m_depthCamparisonOp(Renderer::DEPTH_COMPARE_ALWAYS)
	, m_writeDepth(false)
	, m_colorBlendOp(Renderer::BLEND_OP_ADD)
	, m_colorSrcFactor(Renderer::BLEND_FACTOR_SOURCE_ALPHA)
	, m_colorDstFactor(Renderer::BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA)
	, m_alphaBlendOp(Renderer::BLEND_OP_ADD)
	, m_alphaSrcFactor(Renderer::BLEND_FACTOR_SOURCE_ALPHA)
	, m_alphaDstFactor(Renderer::BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA)
{
}



Shader::Shader()
	: m_shaderProgram(nullptr)
	, m_renderState()
	, m_renderQueue(RENDER_QUEUE_OPAQUE)
{

}



Shader::~Shader()
{
}



Shader::Shader(const XMLElement& definition)
{
	const XMLElement* orderingElement = definition.FirstChildElement("Ordering");
	if (orderingElement)
	{
		if (DoesXMLAttributeExist(*orderingElement, "layer"))
		{
			int layer = ParseXmlAttribute(*orderingElement, "layer", 0);
			if (layer < 0)
			{
				layer = 0;
			}
			m_renderLayer = layer;
		}

		if (DoesXMLAttributeExist(*orderingElement, "queue"))
		{
			std::string queue = ParseXmlAttribute(*orderingElement, "queue", "");
			m_renderQueue = StringToRenderQueue(queue, RENDER_QUEUE_OPAQUE);
		}
	}


	const XMLElement* programElement = definition.FirstChildElement("Program");
	if (programElement)
	{
		
		std::string defines = ParseXmlAttribute(*programElement, "defines", "");

		if (DoesXMLAttributeExist(*programElement, "name"))
		{
			std::string shaderProgramName = ParseXmlAttribute(*programElement, "name", "");
			m_shaderProgram = ShaderProgram::CreateOrGet(shaderProgramName, defines);
		}
		else
		{
			if (DoesXMLAttributeExist(*programElement, "vertex") &&
				DoesXMLAttributeExist(*programElement, "fragment"))
			{
				std::string vertexFilepath   = ParseXmlAttribute(*programElement, "vertex", "");
				std::string fragmentFilepath = ParseXmlAttribute(*programElement, "fragment", "");

				std::string shaderProgramName = ParseXmlAttribute(definition, "name", "");
				shaderProgramName += "__FROM_VS_FS";
				m_shaderProgram = ShaderProgram::CreateOrGet(shaderProgramName, vertexFilepath, fragmentFilepath, defines);
			}
		}
	}

	const XMLElement* fillElement = definition.FirstChildElement("Fill");
	if (fillElement)
	{
		if (DoesXMLAttributeExist(*fillElement, "mode"))
		{
			std::string fillMode = ParseXmlAttribute(*fillElement, "mode", "");
			m_renderState.m_fillMode = Renderer::StringToFillMode(fillMode, m_renderState.m_fillMode);
		}
	}

	const XMLElement* cullElement = definition.FirstChildElement("Cull");
	if (cullElement)
	{
		// CullMode
		if (DoesXMLAttributeExist(*cullElement, "mode"))
		{
			std::string cullMode = ParseXmlAttribute(*cullElement, "mode", "");
			m_renderState.m_cullMode = Renderer::StringToCullMode(cullMode, m_renderState.m_cullMode);
		}

		// Winding Order
		if (DoesXMLAttributeExist(*cullElement, "frontFace"))
		{
			std::string frontFace = ParseXmlAttribute(*cullElement, "frontFace", "");
			m_renderState.m_windingOrder = Renderer::StringToWindingOrder(frontFace, m_renderState.m_windingOrder);
		}
	}

	// Depth
	const XMLElement* depthElement = definition.FirstChildElement("Depth");
	if (depthElement)
	{
		m_renderState.m_writeDepth = ParseXmlAttribute(*depthElement, "write", m_renderState.m_writeDepth);

		if (DoesXMLAttributeExist(*depthElement, "test"))
		{
			std::string depthTest = ParseXmlAttribute(*depthElement, "test", "");
			m_renderState.m_depthCamparisonOp = Renderer::StringToDepthTest(depthTest, m_renderState.m_depthCamparisonOp);
		}
	}

	// Blend
	const XMLElement* blendElement = definition.FirstChildElement("Blend");
	if (blendElement)
	{
		// Color
		const XMLElement* colorElement = definition.FirstChildElement("Color");
		if (colorElement)
		{
			if (DoesXMLAttributeExist(*colorElement, "op"))
			{
				std::string op = ParseXmlAttribute(*colorElement, "op", "");
				m_renderState.m_colorBlendOp = Renderer::StringToBlendOp(op, m_renderState.m_colorBlendOp);
			}

			if (DoesXMLAttributeExist(*colorElement, "src"))
			{
				std::string src = ParseXmlAttribute(*colorElement, "src", "");
				m_renderState.m_colorSrcFactor = Renderer::StringToBlendFactor(src, m_renderState.m_colorSrcFactor);
			}

			if (DoesXMLAttributeExist(*colorElement, "dst"))
			{
				std::string dst = ParseXmlAttribute(*colorElement, "dst", "");
				m_renderState.m_colorSrcFactor = Renderer::StringToBlendFactor(dst, m_renderState.m_colorSrcFactor);
			}
		}


		// Alpha
		const XMLElement* alphaElement = definition.FirstChildElement("Alpha");
		if (alphaElement)
		{
			if (DoesXMLAttributeExist(*colorElement, "op"))
			{
				std::string op = ParseXmlAttribute(*colorElement, "op", "");
				m_renderState.m_alphaBlendOp = Renderer::StringToBlendOp(op, m_renderState.m_alphaBlendOp);
			}

			if (DoesXMLAttributeExist(*colorElement, "src"))
			{
				std::string src = ParseXmlAttribute(*colorElement, "src", "");
				m_renderState.m_alphaSrcFactor = Renderer::StringToBlendFactor(src, m_renderState.m_alphaSrcFactor);
			}

			if (DoesXMLAttributeExist(*colorElement, "dst"))
			{
				std::string dst = ParseXmlAttribute(*colorElement, "dst", "");
				m_renderState.m_alphaSrcFactor = Renderer::StringToBlendFactor(dst, m_renderState.m_alphaSrcFactor);
			}
		}
	}
}



Shader::Shader(const Shader& toCopy)
{
	m_renderLayer = toCopy.m_renderLayer;
	m_renderQueue = toCopy.m_renderQueue;
	m_shaderProgram = toCopy.m_shaderProgram;
	m_renderState = toCopy.m_renderState;
}



unsigned int Shader::GetRenderLayer() const
{
	return m_renderLayer;
}



RenderQueue Shader::GetRenderQueue() const
{
	return m_renderQueue;
}



const ShaderProgram* Shader::GetShaderProgram() const
{
	return m_shaderProgram;
}



const RenderState& Shader::GetRenderState() const
{
	return m_renderState;
}



void Shader::SetRenderQueue(RenderQueue queue)
{
	m_renderQueue = queue;
}



void Shader::SetRenderState(const RenderState& newRenderState)
{
	m_renderState = newRenderState;
}



void Shader::SetRenderLayer(unsigned int layer)
{
	m_renderLayer = layer;
}



void Shader::SetShaderProgram(const ShaderProgram* program)
{
	m_shaderProgram = program;
}



void Shader::EnableBlending(unsigned int op, unsigned int srcFactor, unsigned int dstFactor)
{
	m_renderState.m_alphaBlendOp = op;
	m_renderState.m_alphaSrcFactor = srcFactor;
	m_renderState.m_alphaDstFactor = dstFactor;
}



void Shader::DisableBlending()
{
	TODO("Disable Blending implementation");
	UNIMPLEMENTED();
}



void Shader::SetDepth(unsigned int op, bool shouldWrite)
{
	m_renderState.m_depthCamparisonOp = op;
	m_renderState.m_writeDepth = shouldWrite;
}



void Shader::DisableDepth()
{
	SetDepth(Renderer::DEPTH_COMPARE_ALWAYS, false);
}



void Shader::SetCullMode(unsigned int cullMode)
{
	m_renderState.m_cullMode = cullMode;
}



void Shader::SetFillMode(unsigned int fillMode)
{
	m_renderState.m_fillMode = fillMode;
}



void Shader::SetWindingOrderForFrontFace(unsigned int windOrder)
{
	m_renderState.m_windingOrder = windOrder;
}



bool Shader::IsLit() const
{
	bool isLit = false;

	const ShaderProgram* shaderProgram = GetShaderProgram();
	if (shaderProgram != nullptr && shaderProgram->IsLit())
	{
		isLit = true;
	}

	return isLit;
}



void Shader::ParseShaders(const std::string& shaderFile)
{
	XMLDocument doc;
	doc.LoadFile(shaderFile.c_str());
	XMLElement* shadersElement = doc.FirstChildElement("Shaders");
	

	const XMLElement* shaderElement = shadersElement->FirstChildElement("Shader");
	while(shaderElement != nullptr)
	{
		std::string name = ParseXmlAttribute(*shaderElement, "name", "");
		if (!name.empty())
		{
			// If we have a name.
			auto locationInDefinitions = s_loadedShaders.find(name);
			if (locationInDefinitions == s_loadedShaders.end())
			{
				// And we are not already defined.
				Shader* shader = new Shader(*shaderElement);
				s_loadedShaders[name] = shader;
			}
		}

		shaderElement = shaderElement->NextSiblingElement("Shader");
	}



	doc.Clear();
}



Shader*	Shader::Get(const std::string& name)
{
	Shader* shader = nullptr;

	auto locationInMap = s_loadedShaders.find(name);
	if (locationInMap != s_loadedShaders.end())
	{
		shader = locationInMap->second;
	}

	return shader;
}



Shader* Shader::Clone(const std::string& nameToClone, const std::string& name)
{
	// Clone the shader
	Shader* shader = nullptr;
	Shader* toClone = Get(nameToClone);
	if (toClone != nullptr)
	{
		shader = new Shader(*toClone);
	}
	else
	{
		//shader = Shader(s_defaultShader);
	}


	// Register the new shader if requested
	if (!name.empty())
	{
		// We want to register
		auto locationInMap = s_loadedShaders.find(name);
		if (locationInMap == s_loadedShaders.end())
		{
			s_loadedShaders[name] = shader;
		}
	}

	return shader;
}



std::map<std::string, Shader*> Shader::s_loadedShaders;
