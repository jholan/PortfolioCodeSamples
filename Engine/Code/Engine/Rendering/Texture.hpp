//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include "Engine/Math/IntVector2.hpp"

#include <string>
#include <map>

#include "Engine/Core/Image.hpp"


class Texture
{
	friend class Renderer; // Textures are managed by a Renderer instance

public:
	bool		 IsValid() const;
	unsigned int GetHandle() const;
	IntVector2	 GetDimensions() const;
	Image		 CopyToImage() const;
	void		 CopyFromTexture(const Texture* src);

	static const Texture* CreateOrGet(const std::string& filepath, bool generateMipmaps = false);


private:
	Texture();
	Texture( const Image& image, bool generateMipmaps = false);
	Texture( const std::string& imageFilePath, bool generateMipmaps = false ); // Use CreateOrGetTexture() instead!
	void PopulateFromData( const unsigned char* imageData, const IntVector2& texelSize, int numComponents, bool generateMipmaps = false );
	bool CreateRenderTarget( unsigned int width, unsigned int height, unsigned int textureFormat );


private:
	bool			m_isValid = true;
	unsigned int	m_textureID;
	IntVector2		m_dimensions;
	unsigned int	m_format;


	static std::map<std::string, Texture*> s_loadedTextures;

	static constexpr char* BAD_FILEPATH = "Data/BuiltIns/BAD_TEXTURE.png";
};


