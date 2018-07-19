#pragma once

#include "Engine/Math/AABB2D.hpp"
#include "Engine/Math/IntVector2.hpp"

class Texture;
class Sprite;

class SpriteSheet
{
public:
	SpriteSheet();
	SpriteSheet( const Texture* texture, int tilesWide, int tilesHigh );
	AABB2D GetTexCoordsForSpriteCoords( const IntVector2& spriteCoords ) const;	 // for sprites
	AABB2D GetTexCoordsForSpriteIndex( int spriteIndex ) const;					 // for sprite animations
	int GetNumSprites() const;
	IntVector2 GetSpriteSheetLayout() const;
	const Texture* GetTexture() const;
	
	Sprite* CreateSprite( const IntVector2& spriteCoords, const Vector2& pivot ) const;


private:
	const Texture* 	m_spriteSheetTexture; 	// Texture w/grid-based layout of sprites
	IntVector2		m_spriteLayout;			// # of sprites across, and down, on the sheet
											//... and other data member variables as 
};