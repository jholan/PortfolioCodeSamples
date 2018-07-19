#include "Engine/Rendering/BitmapFont.hpp"

#include "Engine/Rendering/SpriteSheet.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/Texture.hpp"

extern Renderer* g_theRenderer;


//-------------------------------------------------------------------------------------------------------
BitmapFont::BitmapFont(const std::string& fontName, const SpriteSheet glyphSheet, float baseAspect) 
	: m_spriteSheet(glyphSheet)
	, m_baseAspect(baseAspect)
{
	UNUSED(fontName);
}


//-------------------------------------------------------------------------------------------------------
const Texture* BitmapFont::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}


//-------------------------------------------------------------------------------------------------------
AABB2D BitmapFont::GetUVsForGlyph( int glyphUnicode ) const // pass ‘A’ or 65 for A, etc.
{
	AABB2D glyphUVs = m_spriteSheet.GetTexCoordsForSpriteIndex(glyphUnicode);
	return glyphUVs;
}


//-------------------------------------------------------------------------------------------------------
float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	UNUSED(glyphUnicode);
	return m_baseAspect; 
}


//-------------------------------------------------------------------------------------------------------
float BitmapFont::GetStringWidth( const std::string& asciiText, float cellHeight, float aspectScale ) const
{
	float letterWidth = cellHeight * (m_baseAspect * aspectScale);
	float stringWidth = asciiText.length() * letterWidth;

	return stringWidth;
}



const BitmapFont* BitmapFont::CreateOrGet(const std::string& filepath, float baseAspect)
{
	BitmapFont* font = nullptr;

	std::map<std::string, BitmapFont*>::iterator alreadyExists = s_loadedFonts.find(filepath);
	if (alreadyExists == s_loadedFonts.end())
	{
		// The font doesn't exist.
		const Texture* fontTexture = Texture::CreateOrGet(filepath, Texture::TEXTURE_FORMAT_RGBA8);
		if (!fontTexture->IsValid())
		{
			fontTexture = Texture::CreateOrGet(BAD_FILEPATH, Texture::TEXTURE_FORMAT_RGBA8);
		}
		SpriteSheet fontSheet = SpriteSheet(fontTexture, 16, 16);

		font = new BitmapFont(filepath, fontSheet, baseAspect);
		s_loadedFonts[filepath] = font;
	}
	else
	{
		// The font does exist.
		font = s_loadedFonts[filepath];
	}

	return font;
}



std::map<std::string, BitmapFont*> BitmapFont::s_loadedFonts;
