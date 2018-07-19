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
Texture::Texture( const Image& image, bool generateMipmaps ) 
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
	, m_format( NULL )
{
	PopulateFromData(image.GetTexelDataAsUChars(), image.GetDimensions(), 4, generateMipmaps);
}


//-----------------------------------------------------------------------------------------------
Texture::Texture( const std::string& imageFilePath, bool generateMipmaps )
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
	, m_format(NULL)
{
	Image tmpImage = Image(imageFilePath);
	PopulateFromData(tmpImage.GetTexelDataAsUChars(), tmpImage.GetDimensions(), 4, generateMipmaps);
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
void Texture::PopulateFromData( const unsigned char* imageData, const IntVector2& texelSize, int numComponents, bool generateMipmaps )
{
	m_dimensions = texelSize;

	glActiveTexture( GL_TEXTURE0 ); 

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &m_textureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, m_textureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	// Buffer Format
	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( numComponents == 3 )
		bufferFormat = GL_RGB;


	GLenum internalFormat = GL_RGBA8; //bufferFormat; // the format we want the texture to be on the card; allows us to translate into a different texture format as we upload to OpenGL

	int numberOfMipmaps = 1;
	if(generateMipmaps)
	{
		numberOfMipmaps = GetNumberOfMipmapLevels(m_dimensions);
	}

	// create the gpu buffer
	// note: only this is needed for render targets
	glTexStorage2D( 
		GL_TEXTURE_2D,
		numberOfMipmaps,					// number of levels (mip-layers)
		internalFormat,						// how is the memory stored on the GPU
		m_dimensions.x, m_dimensions.y		// dimensions
	);										

	// copies cpu memory to the gpu - needed for texture resources
	glTexSubImage2D( 
		GL_TEXTURE_2D,
		0,									// mip layer we're copying to
		0, 0,								// offset
		m_dimensions.x, m_dimensions.y,		// dimensions
		bufferFormat,						// which channels exist in the CPU buffer
		GL_UNSIGNED_BYTE,					// how are those channels stored
		imageData							// cpu buffer to copy;
	); 


											
	//glTexImage2D(			// Upload this pixel data to our new OpenGL texture
	//	GL_TEXTURE_2D,		// Creating this as a 2d texture
	//	0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
	//	internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
	//	m_dimensions.x,			// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
	//	m_dimensions.y,			// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
	//	0,					// Border size, in texels (must be 0 or 1, recommend 0)
	//	bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
	//	GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
	//	imageData );		// Address of the actual pixel data bytes/buffer in system memory


	if (generateMipmaps)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}


	glBindTexture( GL_TEXTURE_2D, NULL );	// unset it; 
}


//-----------------------------------------------------------------------------------------------
bool Texture::CreateRenderTarget( unsigned int width, unsigned int height, unsigned int textureFormat )
{
	// generate the link to this texture
	glGenTextures( 1, &m_textureID ); 
	if (m_textureID == NULL) 
	{
		return false; 
	}

	// TODO - add a TextureFormatToGLFormats( GLenum*, GLenum*, GLenum*, eTextureFormat )
	//        when more texture formats are required; 
	GLenum internal_format = GL_RGBA8; 
	GLenum channels = GL_RGBA;  
	GLenum pixel_layout = GL_UNSIGNED_BYTE;  
	if (textureFormat == Renderer::TEXTURE_FORMAT_D24S8) 
	{
		internal_format = GL_DEPTH_STENCIL; 
		channels = GL_DEPTH_STENCIL; 
		pixel_layout = GL_UNSIGNED_INT_24_8; 
	}

	// Copy the texture - first, get use to be using texture unit 0 for this; 
	glActiveTexture( GL_TEXTURE0 ); 
	glBindTexture( GL_TEXTURE_2D, m_textureID );    // bind our texture to our current texture unit (0)

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	// Copy data into it;
	glTexImage2D( GL_TEXTURE_2D, 0, 
		internal_format, // what's the format OpenGL should use
		width, 
		height,        
		0,             // border, use 0
		channels,      // how many channels are there?
		pixel_layout,  // how is the data laid out
		nullptr );     // don't need to pass it initialization data 


	// make sure it succeeded
	// cleanup after myself; 
	glBindTexture( GL_TEXTURE_2D, NULL ); // unset it; 

	// Save this all off
	m_dimensions = IntVector2( (int)width, (int)height );
	m_format = textureFormat; // I save the format with the texture
							  // for sanity checking.
	
	// great, success
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

	// texture_data must be large enough to hold the entire texture, in this case
	unsigned char* data = new unsigned char[GetDimensions().x * GetDimensions().y * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	copy.SetTexelDataFromUChars(data, GetDimensions().x, GetDimensions().y, 4);

	delete[] data;
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



const Texture* Texture::CreateOrGet(const std::string& filepath, bool generateMipmaps)
{
	Texture* texture = nullptr;

	std::map<std::string, Texture*>::iterator alreadyExists = s_loadedTextures.find(filepath);
	if (alreadyExists == s_loadedTextures.end())
	{
		// The texture doesn't exist.
		texture = new Texture(filepath, generateMipmaps);
		if (texture->GetDimensions() == IntVector2(0, 0))
		{
			delete texture;
			texture = new Texture(BAD_FILEPATH);
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



std::map<std::string, Texture*> Texture::s_loadedTextures;