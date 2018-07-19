#include "Engine/Rendering/TextureCube.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Rendering/GLFunctions.h"



//-----------------------------------------------------------------------------------------------
bool TextureCube::IsValid() const
{
	return m_isValid;
}


//-----------------------------------------------------------------------------------------------
unsigned int TextureCube::GetHandle() const
{
	return m_textureID;
}


//-----------------------------------------------------------------------------------------------
IntVector2 TextureCube::GetSideDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
TextureCube::TextureCube()
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
{

}


//-----------------------------------------------------------------------------------------------
TextureCube::TextureCube(const Image& image)
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
{
	UNUSED(image);
}


//-----------------------------------------------------------------------------------------------
TextureCube::TextureCube(const std::string& imageFilepath)
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
{
	Image tmpImage = Image(imageFilepath);
}


//-----------------------------------------------------------------------------------------------
void BindImageToFace(const Image* image, unsigned int face)
{
	IntVector2 dimensions = image->GetDimensions();
	glTexImage2D(
		face, 
		0, 
		GL_RGBA, 
		dimensions.x, 
		dimensions.y, 
		0, 
		GL_RGBA, 
		GL_UNSIGNED_BYTE,
		image->GetTexelDataAsUChars()
	);
}


//-----------------------------------------------------------------------------------------------
TextureCube::TextureCube(const Image** images)
{
	// Error checking
	m_dimensions = images[0]->GetDimensions();
	GUARANTEE_OR_DIE(m_dimensions.x == m_dimensions.y, "TextureCube faces must be square");
	int numCubeSides = 6;
	for (int i = 1; i < numCubeSides; ++i)
	{
		IntVector2 sideDim = images[i]->GetDimensions();
		GUARANTEE_OR_DIE(sideDim == m_dimensions, "All TextureCube faces must have the same dimensions");
	}


	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Generate handle
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

	// Set up sampler params
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  


	// Bind Images to the cube map
	BindImageToFace(images[0], GL_TEXTURE_CUBE_MAP_NEGATIVE_X); // left
	BindImageToFace(images[1], GL_TEXTURE_CUBE_MAP_POSITIVE_Z); // front
	BindImageToFace(images[2], GL_TEXTURE_CUBE_MAP_POSITIVE_X); // right
	BindImageToFace(images[3], GL_TEXTURE_CUBE_MAP_NEGATIVE_Z); // back
	BindImageToFace(images[4], GL_TEXTURE_CUBE_MAP_POSITIVE_Y); // top
	BindImageToFace(images[5], GL_TEXTURE_CUBE_MAP_NEGATIVE_Y); // bottom
}