#include "Engine/Rendering/SpriteSheet.hpp"

#include "Engine/Math/MathUtils.hpp"

#include "Engine/Rendering/Sprite.hpp"
#include "Engine/Rendering/Texture.hpp"


//-------------------------------------------------------------------------------
SpriteSheet::SpriteSheet()
	: m_spriteSheetTexture(nullptr)
	, m_spriteLayout(0,0)
{
}



//-------------------------------------------------------------------------------
SpriteSheet::SpriteSheet( const Texture* texture, int tilesWide, int tilesHigh ) 
	: m_spriteSheetTexture(texture)
	, m_spriteLayout(tilesWide, tilesHigh)
{
}


//-------------------------------------------------------------------------------
AABB2D SpriteSheet::GetTexCoordsForSpriteCoords( const IntVector2& spriteCoords ) const
{
	IntVector2 flippedSpriteCoords = spriteCoords;
	flippedSpriteCoords.y = m_spriteLayout.y - spriteCoords.y - 1;

	Vector2 steps = Vector2(1.0f / (float)m_spriteLayout.x, 1.0f / (float)m_spriteLayout.y);

	Vector2 mins = Vector2((float)flippedSpriteCoords.x * steps.x, (float)flippedSpriteCoords.y * steps.y);
	Vector2 maxs = Vector2(((float)flippedSpriteCoords.x + 1.0f) * steps.x, ((float)flippedSpriteCoords.y + 1.0f) * steps.y);
	return AABB2D(mins, maxs);
}


//-------------------------------------------------------------------------------
AABB2D SpriteSheet::GetTexCoordsForSpriteIndex( int spriteIndex ) const
{
	IntVector2 coords = GetCoordsFromIndex(spriteIndex, m_spriteLayout);
	return GetTexCoordsForSpriteCoords(coords);
}


//-------------------------------------------------------------------------------
int SpriteSheet::GetNumSprites() const
{
	int numSprites = m_spriteLayout.x * m_spriteLayout.y;
	return numSprites;
}


//-------------------------------------------------------------------------------
IntVector2 SpriteSheet::GetSpriteSheetLayout() const
{
	return m_spriteLayout;
}


//-------------------------------------------------------------------------------
const Texture* SpriteSheet::GetTexture() const
{
	return m_spriteSheetTexture;
}


//-------------------------------------------------------------------------------
Sprite* SpriteSheet::CreateSprite( const IntVector2& spriteCoords, const Vector2& pivot ) const
{
	AABB2D uvs = GetTexCoordsForSpriteCoords(spriteCoords);

	IntVector2 textureDimensions = m_spriteSheetTexture->GetDimensions();
	IntVector2 spriteDimensions = IntVector2(textureDimensions.x / m_spriteLayout.x, textureDimensions.y / m_spriteLayout.y);
	Vector2 worldDim = Vector2(1.0f, (float)spriteDimensions.y / (float)spriteDimensions.x); // NOTE: Temporary, also if your dumb enough to build a SS with dim.x = 0 you deserve it
	
	Sprite* sprite = new Sprite(GetTexture(), uvs, pivot, worldDim);
	return sprite;
}
