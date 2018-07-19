//-----------------------------------------------------------------------------------------------
// Texture.cpp
//
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Engine/Math/MathUtils.hpp"

#include "Engine/Rendering/GLFunctions.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Rendering/Texture.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/Image.hpp"
#include "Engine/Rendering/Renderer.hpp"



//-----------------------------------------------------------------------------------------------
// Called only by the Renderer.  Use CreateOrGetTexture() to instantiate textures.
//
Texture::Texture()
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
	, m_format( NULL )
{

}


//-----------------------------------------------------------------------------------------------
Texture::Texture( const Image& image, unsigned int textureFormat, bool generateMipmaps ) 
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
	, m_format( NULL )
{
	PopulateFromData(image.GetTexelData(), image.GetDimensions(), textureFormat, generateMipmaps);
}


//-----------------------------------------------------------------------------------------------
Texture::Texture( const std::string& imageFilePath, unsigned int textureFormat, bool generateMipmaps )
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
	, m_format(NULL)
{
	Image tmpImage = Image(imageFilePath);
	PopulateFromData(tmpImage.GetTexelData(), tmpImage.GetDimensions(), textureFormat, generateMipmaps);
}


//-----------------------------------------------------------------------------------------------
int GetNumberOfMipmapLevels(const IntVector2& dimensions)
{
	int maxDim = Max(dimensions.x, dimensions.y);
	int numberOfMipmaps = (int)(Floor(Log2((float)maxDim)) + 1);

	return numberOfMipmaps;
}


//-----------------------------------------------------------------------------------------------
// Creates a texture identity on the video card, and populates it with the given image texel data
//
void Texture::PopulateFromData( const void* imageData, const IntVector2& dimensions, unsigned int textureFormat, bool generateMipmaps )
{
	m_dimensions = dimensions;


	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );


	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glActiveTexture( GL_TEXTURE0 ); 
	glGenTextures( 1, (GLuint*) &m_textureID );
	glBindTexture( GL_TEXTURE_2D, m_textureID );


	// Texture Parameters, COMPLETELY OVERRIDEN BY SAMPLERS
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 


	GLenum internalFormat = textureFormat;
	GLenum layout;
	GLenum type;
	GetLayoutAndTypeFromFormat(internalFormat, layout, type);


	// Determine the number of mipmap levels we need
	int numberOfMipmaps = 1;
	if(generateMipmaps)
	{
		numberOfMipmaps = GetNumberOfMipmapLevels(m_dimensions);
	}


	// Malloc the GPU buffer
	// note: only this is needed for render targets
	glTexStorage2D( 
		GL_TEXTURE_2D,
		numberOfMipmaps,				
		internalFormat,					
		m_dimensions.x, m_dimensions.y
	);										


	// Copies CPU data to the GPU buffer
	glTexSubImage2D( 
		GL_TEXTURE_2D,
		0,									// mip level
		0, 0,								// offset
		m_dimensions.x, m_dimensions.y,		// dimensions
		layout,								
		type,								
		imageData							// CPU data
	); 


	// Generate the mipmaps 
	if (generateMipmaps)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}


	// unset it 
	glBindTexture( GL_TEXTURE_2D, NULL );	
}


//-----------------------------------------------------------------------------------------------
bool Texture::InitializeAsRenderTarget( unsigned int width, unsigned int height, unsigned int textureFormat )
{
	// Generate the texture handle
	glGenTextures( 1, &m_textureID ); 
	if (m_textureID == NULL) 
	{
		return false; 
	}


	// Save this all off
	m_dimensions = IntVector2( (int)width, (int)height );
	m_format = textureFormat;

	
	// Copy the texture - first, get use to be using texture unit 0 for this; 
	glActiveTexture( GL_TEXTURE0 ); 
	glBindTexture( GL_TEXTURE_2D, m_textureID );


	// Texture Parameters, COMPLETELY OVERRIDEN BY SAMPLERS
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 


	GLenum internalFormat = textureFormat;


	// Malloc the GPU Buffer
	glTexStorage2D( 
		GL_TEXTURE_2D,
		1,				
		internalFormat,					
		m_dimensions.x, m_dimensions.y
	);	


	// unset it 
	glBindTexture( GL_TEXTURE_2D, NULL ); 
	

	return true; 
}


//-----------------------------------------------------------------------------------------------
bool Texture::IsValid() const
{
	return m_isValid;
}


//-----------------------------------------------------------------------------------------------
unsigned int Texture::GetHandle() const
{
	return m_textureID;
}


//-----------------------------------------------------------------------------------------------
IntVector2 Texture::GetDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
Image Texture::CopyToImage() const
{
	Image copy;

	glBindTexture(GL_TEXTURE_2D, GetHandle()); // texture_id was given by glGenTextures

	unsigned int layout;
	unsigned int type;
	GetLayoutAndTypeFromFormat(m_format, layout, type);
	if (type == GL_UNSIGNED_BYTE)
	{
		copy.SetImageType(IMAGE_TYPE_BYTE);

		// texture_data must be large enough to hold the entire texture, in this case
		unsigned char* data = new unsigned char[GetDimensions().x * GetDimensions().y * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		copy.SetTexelDataFromUChars(data, GetDimensions().x, GetDimensions().y, 4);
		delete[] data;
	}
	if (type == GL_FLOAT)
	{
		copy.SetImageType(IMAGE_TYPE_FLOAT);

		// texture_data must be large enough to hold the entire texture, in this case
		float* data = new float[GetDimensions().x * GetDimensions().y * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
		copy.SetTexelDataFromFloats(data, GetDimensions().x, GetDimensions().y, 4);
		delete[] data;
	}

	return copy;
}


//-----------------------------------------------------------------------------------------------
void Texture::CopyFromTexture(const Texture* src)
{
	glCopyImageSubData(	src->GetHandle(), GL_TEXTURE_2D, 0, 0, 0, 0,		// src texture location and copy start point
						GetHandle(),      GL_TEXTURE_2D, 0, 0, 0, 0,		// dst texture location and copy start point
						src->GetDimensions().x, src->GetDimensions().y, 1	// Texture dimensions
						);
}



const Texture* Texture::CreateOrGet(const std::string& filepath, unsigned int textureFormat, bool generateMipmaps)
{
	Texture* texture = nullptr;

	std::map<std::string, Texture*>::iterator alreadyExists = s_loadedTextures.find(filepath);
	if (alreadyExists == s_loadedTextures.end())
	{
		// The texture doesn't exist.
		texture = new Texture(filepath, textureFormat, generateMipmaps);
		if (texture->GetDimensions() == IntVector2(0, 0))
		{
			delete texture;
			texture = new Texture(BAD_FILEPATH, textureFormat);
			texture->m_isValid = false;
		}

		s_loadedTextures[filepath] = texture;
	}
	else
	{
		// The texture does exist.
		texture = s_loadedTextures[filepath];
	}

	return texture;
}



Texture* Texture::CreateRenderTarget(unsigned int width, unsigned int height, unsigned int textureFormat)
{
	Texture* texture = new Texture();

	texture->InitializeAsRenderTarget( width, height, textureFormat );    

	return texture;
}



void Texture::GetLayoutAndTypeFromFormat(unsigned int format, unsigned int& out_layout, unsigned int& out_type)
{
	if (format == TEXTURE_FORMAT_RGB8)
	{
		out_layout = GL_RGB;
		out_type = GL_UNSIGNED_BYTE;
	}
	else if (format == TEXTURE_FORMAT_RGBA8)
	{
		out_layout = GL_RGBA;
		out_type = GL_UNSIGNED_BYTE;
	}
	else if (format == TEXTURE_FORMAT_SRGB8)
	{
		out_layout = GL_RGB;
		out_type = GL_UNSIGNED_BYTE;
	}
	else if (format == TEXTURE_FORMAT_SRGBA8)
	{
		out_layout = GL_RGBA;
		out_type = GL_UNSIGNED_BYTE;
	}
	else if (format == TEXTURE_FORMAT_D24S8)
	{
		out_layout = GL_DEPTH_STENCIL;
		out_type = GL_UNSIGNED_INT_24_8;
	}
	else if (format == TEXTURE_FORMAT_RGB16F)
	{
		out_layout = GL_RGB;
		out_type = GL_FLOAT;
	}
	else if (format == TEXTURE_FORMAT_RGBA16F)
	{
		out_layout = GL_RGBA;
		out_type = GL_FLOAT;
	}
	else 
	{
		GUARANTEE_OR_DIE(false, "Texture::GetLayoutAndTypeFromFormat Invalid Format");
	}
}



unsigned int Texture::StringToTextureFormat(const std::string& name, unsigned int defaultFormat)	
{
	unsigned int format = defaultFormat;

	std::string lowerName = ToLower(name);
	if (lowerName == "rgb8")
	{
		format = TEXTURE_FORMAT_RGB8;
	}
	else if (lowerName == "rgba8")
	{
		format = TEXTURE_FORMAT_RGBA8;
	}
	else if (lowerName == "srgb8")
	{
		format = TEXTURE_FORMAT_SRGB8;
	}
	else if (lowerName == "srgba8")
	{
		format = TEXTURE_FORMAT_SRGBA8;
	}
	else if (lowerName == "d24s8")
	{
		format = TEXTURE_FORMAT_D24S8;
	}
	else if (lowerName == "rgb16f")
	{
		format = TEXTURE_FORMAT_RGB16F;
	}
	else if (lowerName == "rgba16f")
	{
		format = TEXTURE_FORMAT_RGBA16F;
	}

	return format;
}



const unsigned int Texture::TEXTURE_FORMAT_RGB8		= GL_RGB8;
const unsigned int Texture::TEXTURE_FORMAT_RGBA8	= GL_RGBA8;

const unsigned int Texture::TEXTURE_FORMAT_SRGB8	= GL_SRGB8;
const unsigned int Texture::TEXTURE_FORMAT_SRGBA8	= GL_SRGB8_ALPHA8;

const unsigned int Texture::TEXTURE_FORMAT_D24S8	= GL_DEPTH24_STENCIL8;

const unsigned int Texture::TEXTURE_FORMAT_RGB16F	= GL_RGB16F;
const unsigned int Texture::TEXTURE_FORMAT_RGBA16F	= GL_RGBA16F;



std::map<std::string, Texture*> Texture::s_loadedTextures;