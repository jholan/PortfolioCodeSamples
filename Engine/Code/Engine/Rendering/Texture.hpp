//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include "Engine/Math/IntVector2.hpp"

#include <string>
#include <map>

#include "Engine/Core/Image.hpp"


// ----------------------------------------------------------------------------------------------------------------
// Texture Formats FAQ --------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
// Format - Full description Channel Layout, and size/type of each channel
//	 EX - TEXTURE_FORMAT_RGB8 
//		    - 3 channels R,G,B
//			- sizeof each channel == 8 bits
//			- GL Type with 8 bits == Byte
//
// Layout - The channels present and the order they appear in in the data 
//	 EX - TEXTURE_FORMAT_RGB8
//			- 3 channels R,G,B 
//			- first the Red channel, then Green channel, then Blue channel in memory
//
// Type - The type of each channel 
//	 EX - TEXTURE_FORMAT_RGB8 
//			- Each channel is 1 Byte or 8 bits in size
//			- Also implies the values of the data. Byte is an int type so values will be 0 - 255 



class Texture
{
	friend class Renderer; // Textures are managed by a Renderer instance

public:
	bool					IsValid() const;
	unsigned int			GetHandle() const;
	IntVector2				GetDimensions() const;
	Image					CopyToImage() const;
	void					CopyFromTexture(const Texture* src);

	static const Texture*	CreateOrGet(const std::string& filepath, unsigned int textureFormat, bool generateMipmaps = false);
	static Texture*			CreateRenderTarget(unsigned int width, unsigned int height, unsigned int textureFormat);



	// ----------------------------------------------------------------------------------------------------------------
	// Texture Formats ------------------------------------------------------------------------------------------------
	// ----------------------------------------------------------------------------------------------------------------
	static void				GetLayoutAndTypeFromFormat(unsigned int format, unsigned int& out_layout, unsigned int& out_type); // Output is raw GL Types
	static unsigned int		StringToTextureFormat(const std::string& name, unsigned int defaultFormat);																												   //-------------------------------------------------------------------------------------------------------
	

	// RGB
	static const unsigned int TEXTURE_FORMAT_RGB8;
	static const unsigned int TEXTURE_FORMAT_RGBA8;
	
	// sRGB
	static const unsigned int TEXTURE_FORMAT_SRGB8;
	static const unsigned int TEXTURE_FORMAT_SRGBA8;

	// Depth
	static const unsigned int TEXTURE_FORMAT_D24S8;

	// Float
	static const unsigned int TEXTURE_FORMAT_RGB16F;
	static const unsigned int TEXTURE_FORMAT_RGBA16F;



private:
	Texture();
	Texture( const Image& image, unsigned int textureFormat, bool generateMipmaps = false);
	Texture( const std::string& imageFilePath, unsigned int textureFormat, bool generateMipmaps = false ); // Use CreateOrGetTexture() instead!
	void PopulateFromData( const void* imageData, const IntVector2& texelSize, unsigned int textureFormat, bool generateMipmaps = false );
	bool InitializeAsRenderTarget( unsigned int width, unsigned int height, unsigned int textureFormat );


private:
	bool			m_isValid = true;
	unsigned int	m_textureID;
	IntVector2		m_dimensions;
	unsigned int	m_format;


	static std::map<std::string, Texture*> s_loadedTextures;

	static constexpr char* BAD_FILEPATH = "Data/BuiltIns/BAD_TEXTURE.png";
};


