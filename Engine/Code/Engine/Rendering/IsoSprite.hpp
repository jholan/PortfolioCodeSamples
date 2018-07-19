#pragma once

#include <map>
#include <string>

#include "Engine/Core/XmlUtilities.hpp"

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

#define NUM_SPRITES 4

class Sprite;



class IsoSprite
{
public:
	IsoSprite(const Sprite* awayLeft, const Sprite* awayRight, const Sprite* towardRight, const Sprite* towardLeft);
	IsoSprite(const XMLElement& definition);
	~IsoSprite();

	const Sprite* GetSpriteForView(const Vector3& spriteForward, const Vector3& spriteRight, const Vector3& viewerForward, Vector2* out_scale = nullptr) const;


	static IsoSprite* Get( const std::string& name );
	static void		  LoadIsoSpritesFromXML( const std::string& filepath );

private:
	const Sprite* m_awayLeft	= nullptr;
	const Sprite* m_awayRight	= nullptr;
	const Sprite* m_towardRight	= nullptr;
	const Sprite* m_towardLeft	= nullptr;

	Vector2 m_spriteScales[NUM_SPRITES];

	static const int AL = 0;
	static const int AR = 1;
	static const int TL = 2;
	static const int TR = 3;
	static const Vector2 DEFAULT_SPRITE_SCALE;

	// Database
	static std::map<std::string, IsoSprite*> s_loadedIsoSprites;

	static constexpr char* BAD_FILEPATH = "Data/BuiltIns/BAD_ISOSPRITE.png";
	static const Sprite*   BAD_SPRITE;
};