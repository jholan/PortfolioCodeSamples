#pragma once

#include <vector>

#include "Engine/Core/XmlUtilities.hpp"

#include "Engine/Math/AABB2D.hpp"

class Renderer;
class SpriteSheet;
class Texture;



enum SpriteAnimMode
{
	SPRITE_ANIM_MODE_INVALID = -1,

	SPRITE_ANIM_MODE_PLAY_TO_END = 0,	// Play from time=0 to durationSeconds, then finish
	SPRITE_ANIM_MODE_LOOPING,		    // Play from time=0 to end then repeat (never finish)
	//SPRITE_ANIM_MODE_PINGPONG, 		// optional, play forwards, backwards, forwards...

	SPRITE_ANIM_MODE_COUNT
};


class SpriteAnimDefinition
{
public:
	// Composition
	SpriteAnimDefinition(const XMLElement& definition, SpriteSheet* setSpriteSheet = nullptr, float setFPS =-1.0f);
	SpriteAnimDefinition(const std::string& name, SpriteSheet* spriteSheet, float duration, bool isLooping, std::vector<int> indices);
	SpriteAnimDefinition(const std::string& name, SpriteSheet* spriteSheet, float duration, bool isLooping, int startingSpriteIndex, int numberOfFrames);
	~SpriteAnimDefinition();

	// Rendering
	const Texture*  GetTexture() const;
	AABB2D			GetTexCoordsForFractionCompleted(float fraction) const;
	 
	// Name
	const std::string& GetName() const;
	
	// Anim Data
	float		   GetFrameTimeFraction() const;
	float		   GetDurationSeconds() const;
	SpriteAnimMode GetPlaybackMode() const;
	bool		   IsAutoOrient() const;

private:
	void NonXMLInitialize(const std::string& name, SpriteSheet* spriteSheet, float duration, bool isLooping, std::vector<int> indices);

	std::string		 m_name;
	bool			 m_usingDefaultSpriteSheet = false;
	SpriteSheet*	 m_spriteSheet = nullptr;
	float			 m_durationSeconds = 1.0f;
	float			 m_frameTimeFraction;
	SpriteAnimMode	 m_playbackMode = SPRITE_ANIM_MODE_PLAY_TO_END;
	std::vector<int> m_spriteIndices;
	bool			 m_isAutoOrient = false;
};