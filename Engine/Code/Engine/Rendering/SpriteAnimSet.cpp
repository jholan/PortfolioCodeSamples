#include "Engine/Rendering/SpriteAnimSet.hpp"

#include <vector>

#include "Engine/Rendering/SpriteAnim.hpp"
#include "Engine/Rendering/SpriteAnimSetDefinition.hpp"



SpriteAnimSet::SpriteAnimSet(SpriteAnimSetDefinition* animSetDef)
{
	m_definition = animSetDef;

	std::vector<SpriteAnim*> anims = m_definition->GetSpriteAnims();
	for (size_t animIndex = 0; animIndex < anims.size(); ++animIndex)
	{
		m_spriteAnimsByName[anims[animIndex]->GetName()] = anims[animIndex];
	}
}



SpriteAnimSet::~SpriteAnimSet()
{
	for (auto animIter = m_spriteAnimsByName.begin(); animIter != m_spriteAnimsByName.end(); ++animIter)
	{
		delete animIter->second;
	}
}



void SpriteAnimSet::Update(float deltaSeconds)
{
	if (m_currentSpriteAnim != nullptr)
	{
		m_currentSpriteAnim->Update(deltaSeconds);
	}
}



const Texture* SpriteAnimSet::GetCurrentAnimTexture() const
{
	const Texture* texture = nullptr;

	if (m_currentSpriteAnim != nullptr)
	{
		texture = m_currentSpriteAnim->GetTexture();
	}

	return texture;
}



AABB2D SpriteAnimSet::GetCurrentAnimTextureCoords() const
{
	AABB2D textureCoords;

	if (m_currentSpriteAnim != nullptr)
	{
		textureCoords = m_currentSpriteAnim->GetCurrentTexCoords();
	}

	return textureCoords;
}



void SpriteAnimSet::StartAnim(const std::string& animName)
{
	auto locationInMap = m_spriteAnimsByName.find(animName);
	if (locationInMap != m_spriteAnimsByName.end())
	{
		if (locationInMap->second != m_currentSpriteAnim)
		{
			m_currentSpriteAnim = locationInMap->second;
			m_currentSpriteAnim->Reset();
		}
	}
}



SpriteAnim* SpriteAnimSet::GetCurrentAnim()
{
	return m_currentSpriteAnim;
}
