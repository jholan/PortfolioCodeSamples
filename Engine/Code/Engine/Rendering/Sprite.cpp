#include "Engine/Rendering/Sprite.hpp"

#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.h"

#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/SpriteSheet.hpp"

extern Renderer* g_theRenderer;


Sprite::Sprite(const Texture* texture, const AABB2D& uvs, const Vector2& pivot, const Vector2& worldDimensions)
	: m_pivot(pivot)
	, m_worldDimensions(worldDimensions)
	, m_texture(texture)
	, m_UVCoords(uvs)
{

}



Sprite::Sprite(const XMLElement& definition, bool flippedY)
{

	// We really need a texture guys
	const XMLElement* diffuseTextureElement = definition.FirstChildElement("diffuse");
	GUARANTEE_OR_DIE(diffuseTextureElement != nullptr, Stringf("Sprite %s MUST specify a diffuse texture\n", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	if (diffuseTextureElement)
	{
		std::string textureFilepath = ParseXmlAttribute(*diffuseTextureElement, "filepath", "");
		m_texture = Texture::CreateOrGet(textureFilepath);
		GUARANTEE_OR_DIE(m_texture != nullptr, Stringf("Sprite %s MUST specify a valid diffuse texture\n", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
	}

	// Get the pivot if specified 
	const XMLElement* pivotElement = definition.FirstChildElement("pivot");
	if (pivotElement)
	{
		m_pivot = ParseXmlAttribute(*pivotElement, "xy", m_pivot);
	}

	// Get the ppu if it is specified otherwise use the width of the texture
	int ppu = m_texture->GetDimensions().x;
	const XMLElement* ppuElement = definition.FirstChildElement("ppu");
	if (ppuElement)
	{
		ppu = ParseXmlAttribute(*ppuElement, "count", ppu);
	}

	// If they specified a way to calculate uvs
	const XMLElement* uvElement = definition.FirstChildElement("uv");
	if (uvElement)
	{
		// Check which way
		// If it was pixel coordinates
		if (DoesXMLAttributeExist(*uvElement, "pixelCoordinates"))
		{
			// Get the coords
			std::string rawPixelCoords = ParseXmlAttribute(*uvElement, "pixelCoordinates", "");
			std::vector<int> pixelCoords = ParseCSVtoInts(rawPixelCoords);
			GUARANTEE_OR_DIE( pixelCoords.size() == 4, Stringf("To use pixel coordinates in Sprite %s please specify 4 int values\n", ParseXmlAttribute(definition, "name", "").c_str()).c_str());
			
			// Get our uv rect
			Vector2 textureDimensions = (Vector2)m_texture->GetDimensions();
			Vector2 lowerLeft  = Vector2((float)pixelCoords[0], (float)pixelCoords[1]);
			Vector2 upperRight = Vector2((float)pixelCoords[2], (float)pixelCoords[3]);
			if (flippedY)
			{
				// Flip the y values if necessary
				float temp = lowerLeft.y;
				lowerLeft.y  = upperRight.y;
				upperRight.y = temp;
			}

			// Calculate the world dimensions
			Vector2 spriteDimensions = upperRight - lowerLeft;
			spriteDimensions.x = abs(spriteDimensions.x);
			spriteDimensions.y = abs(spriteDimensions.y);
			m_worldDimensions = spriteDimensions / (float)ppu;

			// Calculate the uvs
			m_UVCoords.mins = Vector2(lowerLeft.x / textureDimensions.x,  1.0f - (lowerLeft.y / textureDimensions.y));
			m_UVCoords.maxs = Vector2(upperRight.x / textureDimensions.x, 1.0f - (upperRight.y / textureDimensions.y));
		}
	}

}



Sprite::~Sprite()
{

}



Vector2 Sprite::GetPivot() const
{
	return m_pivot;
}



Vector2 Sprite::GetWorldDimensions() const
{
	return m_worldDimensions;
}



const Texture* Sprite::GetTexture() const
{
	return m_texture;
}



AABB2D Sprite::GetUVCoordinates() const
{
	return m_UVCoords;
}



Sprite* Sprite::Get( const std::string& name )
{
	Sprite* sprite = nullptr;

	std::map<std::string, Sprite*>::iterator alreadyExists = s_loadedSprites.find(name);
	if (alreadyExists == s_loadedSprites.end())
	{
		// The sprite doesn't exist.
		sprite = new Sprite(Texture::CreateOrGet(BAD_FILEPATH), AABB2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f)), Vector2(0.5f, 0.5f), Vector2(1.0f, 1.0f));
		s_loadedSprites[name] = sprite;
	}
	else
	{
		// The sprite exists.
		sprite = s_loadedSprites[name];
	}

	return sprite;
}



void Sprite::LoadSpritesFromXML( const std::string& filepath )
{
	XMLDocument doc;
	doc.LoadFile( filepath.c_str() );
	XMLElement* definitions = doc.FirstChildElement("sprites");
	bool flippedY = ParseXmlAttribute(*definitions, "flippedY", false);

	const XMLElement* spriteElement = definitions->FirstChildElement("sprite");
	while (spriteElement != nullptr)
	{
		std::string spriteName = ParseXmlAttribute(*spriteElement, "name", "");
		if (!spriteName.empty())
		{
			// If we have a name.
			auto locationInDatabase = s_loadedSprites.find(spriteName);
			if (locationInDatabase == s_loadedSprites.end())
			{
				// And we are not already defined.
				Sprite* newSprite = new Sprite(*spriteElement, flippedY);
				s_loadedSprites[spriteName] = newSprite;
			}
		}

		spriteElement = spriteElement->NextSiblingElement("sprite");
	}

	doc.Clear();
}



void Sprite::LoadSpritesFromSpriteSheet( SpriteSheet* spriteSheet, const std::string& sheetName)
{
	// Short circuit
	if (spriteSheet == nullptr)
	{
		return;
	}

	IntVector2 spriteSheetDimensions = spriteSheet->GetSpriteSheetLayout();
	for (int y = 0; y < spriteSheetDimensions.y; ++y)
	{
		for (int x = 0; x < spriteSheetDimensions.x; ++x)
		{
			// Create our name and look to see if we already exist.
			std::string spriteName = Stringf("%s_%i_%i", sheetName.c_str(), x, y);
			auto locationInDatabase = s_loadedSprites.find(spriteName);
			if (locationInDatabase == s_loadedSprites.end())
			{
				// If we don't.
				s_loadedSprites[spriteName] = spriteSheet->CreateSprite(IntVector2(x, y), SPRITESHEET_DEFAULT_PIVOT);
			}
		}
	}

}



std::map<std::string, Sprite*> Sprite::s_loadedSprites;
const Vector2 Sprite::SPRITESHEET_DEFAULT_PIVOT = Vector2(0.5f, 0.5f);
