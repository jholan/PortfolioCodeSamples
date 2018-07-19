#pragma once

#include <string>
#include <map>

#include "Engine/Math/AABB2D.hpp"

class SpriteAnimSetDefinition;
class SpriteAnim;
class Texture;



class SpriteAnimSet
{
public:
	SpriteAnimSet(SpriteAnimSetDefinition* animSetDef);
	~SpriteAnimSet();

	void Update(float deltaSeconds);
	
	const Texture*  GetCurrentAnimTexture()	   const;
	AABB2D			GetCurrentAnimTextureCoords() const;

	void StartAnim(const std::string& animName);
	SpriteAnim* GetCurrentAnim();


private:
	SpriteAnimSetDefinition* m_definition;

	SpriteAnim*						   m_currentSpriteAnim;
	std::map<std::string, SpriteAnim*> m_spriteAnimsByName;
};