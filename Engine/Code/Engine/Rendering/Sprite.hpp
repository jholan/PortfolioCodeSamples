#pragma once

#include <map>
#include <string>

#include "Engine/Core/XmlUtilities.hpp"

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2D.hpp"

class Texture;
class SpriteSheet;



class Sprite
{
public:
	Sprite(const Texture* texture, const AABB2D& uvs, const Vector2& pivot, const Vector2& worldDimensions);
	Sprite(const XMLElement& definition, bool flippedY = false);
	~Sprite();

	Vector2 GetPivot() const;
	Vector2 GetWorldDimensions() const;

	const Texture*	GetTexture() const;
	AABB2D			GetUVCoordinates() const;

	static Sprite*  Get( const std::string& name );
	static void		LoadSpritesFromXML( const std::string& filepath );
	static void		LoadSpritesFromSpriteSheet( SpriteSheet* spriteSheet, const std::string& sheetName);

private:
	Vector2  m_pivot			= Vector2(0.5f, 0.5f);
	Vector2  m_worldDimensions  = Vector2(1.0f, 1.0f);
	
	const Texture*  m_texture			= nullptr;
	AABB2D			m_UVCoords			= AABB2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));


	// Database
	static std::map<std::string, Sprite*> s_loadedSprites;
	static const Vector2				  SPRITESHEET_DEFAULT_PIVOT;

	static constexpr char* BAD_FILEPATH = "Data/BuiltIns/BAD_SPRITE";
};