#pragma once

#include <map>
#include <string>

#include "Engine/Math/AABB2D.hpp"
#include "Engine/Rendering/SpriteSheet.hpp"



class BitmapFont
{
	friend class Renderer;

public:
	const Texture*  GetTexture() const;
	AABB2D			GetUVsForGlyph( int glyphUnicode ) const; // pass ‘A’ or 65 for A, etc.
	float			GetGlyphAspect( int glyphUnicode ) const; // will change later
	float			GetStringWidth( const std::string& asciiText, float cellHeight, float aspectScale ) const;

	static const BitmapFont* CreateOrGet(const std::string& filepath, float baseAspect = 1.0f);


private:
	BitmapFont(const std::string& fontName, const SpriteSheet glyphSheet, float baseAspect = 1.0f); // constructed by Renderer

private:
	const SpriteSheet	m_spriteSheet; // used internally; assumed to be a 16x16 glyph sheet
	float				m_baseAspect;  // used as the base aspect ratio for all glyphs


	// Database
	static std::map<std::string, BitmapFont*> s_loadedFonts;

	static constexpr char* BAD_FILEPATH = "Data/BuiltIns/BAD_FONT.png";
};