#include "Engine/Rendering/Sampler.hpp"

#include "Engine/Rendering/GLFunctions.h"



//-------------------------------------------------------------------------------------------------------
Sampler::Sampler() 
	: m_samplerHandle(NULL)
{

}


//-------------------------------------------------------------------------------------------------------
Sampler::~Sampler()
{
	Destroy();
}


//-------------------------------------------------------------------------------------------------------
bool Sampler::Initialize(bool isLinear, bool shouldUseMipmaps, const unsigned int wrapMode, const RGBA& borderColor)
{
	// create the sampler handle if needed; 
	if (m_samplerHandle == NULL) {
		glGenSamplers( 1, &m_samplerHandle ); 
		if (m_samplerHandle == NULL) {
			return false; 
		}
	}

	
	// Wrapping
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_S, wrapMode );  
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_T, wrapMode );  
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_R, wrapMode );  


	// Border Color
	Vector4 floatBorderColor = borderColor.GetAsVector4();
	float values[4] = {floatBorderColor.x, floatBorderColor.y, floatBorderColor.z, floatBorderColor.w};
	//float values[4] = {2.0f, 2.0f, 2.0f, 2.0f};
	glSamplerParameterfv(m_samplerHandle, GL_TEXTURE_BORDER_COLOR, values);  

	
	// Filtering 
	GLint minFilterMode = GL_NEAREST;
	GLint magFilterMode = GL_NEAREST;
	if (isLinear)
	{
		if (shouldUseMipmaps)
		{
			// Linear Sampling
			// Linear Mip maps
			minFilterMode = GL_LINEAR_MIPMAP_LINEAR;
			magFilterMode = GL_LINEAR;
		}
		else
		{
			// Linear Sampling
			// No Mip maps
			minFilterMode = GL_LINEAR;
			magFilterMode = GL_LINEAR;
		}
	}
	else
	{
		if (shouldUseMipmaps)
		{
			// Nearest Sampling
			// Linear Mip maps
			minFilterMode = GL_NEAREST_MIPMAP_LINEAR;
			magFilterMode = GL_NEAREST;
		}
		else
		{
			// Nearest Sampling
			// No Mip maps
			minFilterMode = GL_NEAREST;
			magFilterMode = GL_NEAREST;
		}
	}

	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MIN_FILTER, minFilterMode );
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MAG_FILTER, magFilterMode );
	return true; 
}


//-------------------------------------------------------------------------------------------------------
void Sampler::Destroy()
{
	if (m_samplerHandle != NULL) {
		glDeleteSamplers( 1, &m_samplerHandle ); 
		m_samplerHandle = NULL; 
	}
} 


//-------------------------------------------------------------------------------------------------------
unsigned int Sampler::GetSamplerHandle() const
{
	return m_samplerHandle;
}


//-------------------------------------------------------------------------------------------------------
const Sampler* Sampler::GetLinearSampler()
{
	if (LINEAR_SAMPLER == nullptr)
	{
		Sampler* linearSampler = new Sampler();
		linearSampler->Initialize(true);
		LINEAR_SAMPLER = linearSampler;
	}

	return LINEAR_SAMPLER;
}


//-------------------------------------------------------------------------------------------------------
const Sampler* Sampler::GetPointSampler()
{
	if (POINT_SAMPLER == nullptr)
	{
		Sampler* pointSampler = new Sampler();
		pointSampler->Initialize(false);
		POINT_SAMPLER = pointSampler;
	}

	return POINT_SAMPLER;
}


//-------------------------------------------------------------------------------------------------------
const unsigned int Sampler::WRAP_MODE_REPEAT			= GL_REPEAT;
const unsigned int Sampler::WRAP_MODE_CLAMP_TO_EDGE		= GL_CLAMP_TO_EDGE;
const unsigned int Sampler::WRAP_MODE_CLAMP_TO_BORDER	= GL_CLAMP_TO_BORDER;


//-------------------------------------------------------------------------------------------------------
const Sampler* Sampler::LINEAR_SAMPLER	= nullptr;
const Sampler* Sampler::POINT_SAMPLER	= nullptr;