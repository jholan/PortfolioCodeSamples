#include "Engine/Rendering/SpriteAnimSetDefinition.hpp"

#include <string>

#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Math/IntVector2.hpp"

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/SpriteSheet.hpp"
#include "Engine/Rendering/SpriteAnimDefinition.hpp"
#include "Engine/Rendering/SpriteAnim.hpp"
#include "Engine/Rendering/Renderer.hpp"


SpriteAnimSetDefinition::SpriteAnimSetDefinition(const XMLElement& definition)
{
	std::string defaultSpriteSheetTextureName = ParseXmlAttribute(definition, "spriteSheet", "");
	IntVector2 defaultSpriteSheetDimensions = ParseXmlAttribute(definition, "spriteLayout", IntVector2(1, 1));
	if (!defaultSpriteSheetTextureName.empty())
	{
		const Texture* defaultSpriteSheetTexture = Texture::CreateOrGet("Data/Images/" + defaultSpriteSheetTextureName);
		m_defaultSpriteSheet = new SpriteSheet(defaultSpriteSheetTexture, defaultSpriteSheetDimensions.x, defaultSpriteSheetDimensions.y);
	}

	float defualtDuration = ParseXmlAttribute(definition, "duration", -1.0f);
	
	const XMLElement* animDefElement = definition.FirstChildElement("SpriteAnim");
	while(animDefElement != nullptr)
	{
		std::string animDefName = ParseXmlAttribute(*animDefElement, "name", "");
		GUARANTEE_OR_DIE(!animDefName.empty(), "SpriteAnims must have a name");
		SpriteAnimDefinition* animDef = new SpriteAnimDefinition(*animDefElement, m_defaultSpriteSheet, defualtDuration);
		
		auto locationInMap = m_animDefinitionsByName.find(animDefName);
		if (locationInMap == m_animDefinitionsByName.end())
		{
			m_animDefinitionsByName[animDefName] = animDef;
		}

		animDefElement = animDefElement->NextSiblingElement("SpriteAnim");
	}

}



SpriteAnimSetDefinition::~SpriteAnimSetDefinition()
{
	for (auto animDefIter = m_animDefinitionsByName.begin(); animDefIter != m_animDefinitionsByName.end(); ++animDefIter)
	{
		delete animDefIter->second;
	}

	if (m_defaultSpriteSheet != nullptr)
	{
		delete m_defaultSpriteSheet;
	}
}



std::vector<SpriteAnim*> SpriteAnimSetDefinition::GetSpriteAnims() const
{
	std::vector<SpriteAnim*> spriteAnims;

	for (auto animDefIter = m_animDefinitionsByName.begin(); animDefIter != m_animDefinitionsByName.end(); ++animDefIter)
	{
		SpriteAnim* spriteAnim = new SpriteAnim(animDefIter->second);
		spriteAnims.push_back(spriteAnim);
	}

	return spriteAnims;
}
