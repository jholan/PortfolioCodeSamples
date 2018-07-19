#include "Engine/Rendering/IsoSprite.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"

#include "Engine/Math/MathUtils.hpp"

#include "Engine/Rendering/Sprite.hpp"

#include "Engine/Rendering/SpriteSheet.hpp"
#include "Engine/Rendering/Texture.hpp"


IsoSprite::IsoSprite(const Sprite* awayLeft, const Sprite* awayRight, const Sprite* towardRight, const Sprite* towardLeft)
	: m_awayLeft(awayLeft)
	, m_awayRight(awayRight)
	, m_towardRight(towardRight)
	, m_towardLeft(towardLeft)
{
	GUARANTEE_OR_DIE(m_awayLeft    != nullptr &&
					 m_awayRight   != nullptr &&
					 m_towardRight != nullptr &&
					 m_towardLeft  != nullptr, "All sprites for an IsoSprite must be valid");
}



IsoSprite::IsoSprite(const XMLElement& definition)
{
	const XMLElement* al = definition.FirstChildElement("al");
	const XMLElement* ar = definition.FirstChildElement("ar");
	const XMLElement* tl = definition.FirstChildElement("tl");
	const XMLElement* tr = definition.FirstChildElement("tr");

	GUARANTEE_OR_DIE(al != nullptr, Stringf("IsoSprite %s must specify an sprite for away left(al)", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	GUARANTEE_OR_DIE(ar != nullptr, Stringf("IsoSprite %s must specify an sprite for away right(ar)", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	GUARANTEE_OR_DIE(tl != nullptr, Stringf("IsoSprite %s must specify an sprite for toward left(tl)", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	GUARANTEE_OR_DIE(tr != nullptr, Stringf("IsoSprite %s must specify an sprite for toward right(tr)", ParseXmlAttribute(definition, "name", "").c_str()).c_str());

	// AL
	m_awayLeft = Sprite::Get(ParseXmlAttribute(*al, "sprite", "").c_str());
	GUARANTEE_OR_DIE(m_awayLeft != nullptr, Stringf("IsoSprite %s al sprite is not valid", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	m_spriteScales[AL] = ParseXmlAttribute(*al, "scale", DEFAULT_SPRITE_SCALE);

	// AR
	m_awayRight = Sprite::Get(ParseXmlAttribute(*ar, "sprite", "").c_str());
	GUARANTEE_OR_DIE(m_awayRight != nullptr, Stringf("IsoSprite %s ar sprite is not valid", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	m_spriteScales[AR] = ParseXmlAttribute(*ar, "scale", DEFAULT_SPRITE_SCALE);
	
	// TL
	m_towardLeft = Sprite::Get(ParseXmlAttribute(*tl, "sprite", "").c_str());
	GUARANTEE_OR_DIE(m_towardLeft != nullptr, Stringf("IsoSprite %s tl sprite is not valid", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	m_spriteScales[TL] = ParseXmlAttribute(*tl, "scale", DEFAULT_SPRITE_SCALE);
	
	// TR
	m_towardRight = Sprite::Get(ParseXmlAttribute(*tr, "sprite", "").c_str());
	GUARANTEE_OR_DIE(m_towardRight != nullptr, Stringf("IsoSprite %s tr sprite is not valid", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	m_spriteScales[TR] = ParseXmlAttribute(*tr, "scale", DEFAULT_SPRITE_SCALE);

}



IsoSprite::~IsoSprite()
{

}



const Sprite* IsoSprite::GetSpriteForView(const Vector3& spriteForward, const Vector3& spriteRight, const Vector3& viewerForward, Vector2* out_scale) const
{
	float dotFwdView = DotProduct(spriteForward, viewerForward);
	float dotRightView = DotProduct(spriteRight, viewerForward);

	bool isFacingToward = true;
	bool isFacingAway;
	bool isFacingRight = true;
	bool isFacingLeft;

	// Set up bools
	if (dotFwdView >= 0.0f)
	{
		isFacingToward = false;
	}
	isFacingAway = !isFacingToward;

	if (dotRightView < 0.0f)
	{
		isFacingRight = false;
	}
	isFacingLeft = !isFacingRight;


	// Choose sprite
	const Sprite* sprite = nullptr;
	if (isFacingAway && isFacingLeft)
	{
		sprite = m_awayRight;
		if (out_scale != nullptr)
		{
			*out_scale = m_spriteScales[AR];
		}
	}
	else if (isFacingAway && isFacingRight)
	{
		sprite = m_awayLeft;
		if (out_scale != nullptr)
		{
			*out_scale = m_spriteScales[AL];
		}
	}
	else if (isFacingToward && isFacingRight)
	{
		sprite = m_towardLeft;
		if (out_scale != nullptr)
		{
			*out_scale = m_spriteScales[TL];
		}
	}
	else 
	{
		sprite = m_towardRight;
		if (out_scale != nullptr)
		{
			*out_scale = m_spriteScales[TR];
		}
	}
	return sprite;
}



IsoSprite* IsoSprite::Get( const std::string& name )
{
	IsoSprite* isoSprite = nullptr;

	std::map<std::string, IsoSprite*>::iterator alreadyExists = s_loadedIsoSprites.find(name);
	if (alreadyExists == s_loadedIsoSprites.end())
	{
		// The IsoSprite doesn't exist.
		isoSprite = new IsoSprite(BAD_SPRITE, BAD_SPRITE, BAD_SPRITE, BAD_SPRITE);
		s_loadedIsoSprites[name] = isoSprite;
	}
	else
	{
		// The IsoSprite exists.
		isoSprite = s_loadedIsoSprites[name];
	}

	return isoSprite;
}



void IsoSprite::LoadIsoSpritesFromXML( const std::string& filepath )
{
	XMLDocument doc;
	doc.LoadFile( filepath.c_str() );
	XMLElement* definitions = doc.FirstChildElement("isosprites");

	const XMLElement* isoSpriteElement = definitions->FirstChildElement("isosprite");
	while (isoSpriteElement != nullptr)
	{
		std::string isoSpriteName = ParseXmlAttribute(*isoSpriteElement, "name", "");
		if (!isoSpriteName.empty())
		{
			// If we have a name.
			auto locationInDatabase = s_loadedIsoSprites.find(isoSpriteName);
			if (locationInDatabase == s_loadedIsoSprites.end())
			{
				// And we are not already defined.
				IsoSprite* newIsoSprite = new IsoSprite(*isoSpriteElement);
				s_loadedIsoSprites[isoSpriteName] = newIsoSprite;
			}
		}

		isoSpriteElement = isoSpriteElement->NextSiblingElement("isosprite");
	}

	doc.Clear();

	if (BAD_SPRITE == nullptr)
	{
		SpriteSheet ss = SpriteSheet(Texture::CreateOrGet(BAD_FILEPATH), 1, 1);
		BAD_SPRITE = ss.CreateSprite(IntVector2(0,0), Vector2(0.5f, 0.5f));
	}
}



const Vector2 IsoSprite::DEFAULT_SPRITE_SCALE = Vector2(1.0f, 1.0f);
std::map<std::string, IsoSprite*> IsoSprite::s_loadedIsoSprites;
const Sprite* IsoSprite::BAD_SPRITE = nullptr;