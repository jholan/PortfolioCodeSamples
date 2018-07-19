#include "Engine/Rendering/SpriteAnimDefinition.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/IntVector2.hpp"

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/SpriteSheet.hpp"
#include "Engine/Rendering/Renderer.hpp"



SpriteAnimDefinition::SpriteAnimDefinition(const XMLElement& definition, SpriteSheet* setSpriteSheet, float setFPS)
{
	m_name = ParseXmlAttribute(definition, "name", m_name);

	// Double check this it seems jank
	bool durationIsSpecified = DoesXMLAttributeExist(definition, "duration");
	if (durationIsSpecified || (!durationIsSpecified && (setFPS == -1.0f)))
	{
		m_durationSeconds = ParseXmlAttribute(definition, "duration", m_durationSeconds);
	}
	else
	{
		m_durationSeconds = setFPS;
	}

	bool spriteSheetIsSpecified = DoesXMLAttributeExist(definition,"spriteSheet");
	bool spriteLayoutIsSpecified = DoesXMLAttributeExist(definition, "spriteLayout");
	if (spriteSheetIsSpecified && spriteLayoutIsSpecified)
	{
		std::string spriteSheetTextureName = ParseXmlAttribute(definition, "spriteSheet", "");
		spriteSheetTextureName = "Data/Images/" + spriteSheetTextureName;
		const Texture* spriteSheetTexture = Texture::CreateOrGet(spriteSheetTextureName);

		IntVector2 spriteSheetLayout = ParseXmlAttribute(definition, "spriteLayout", IntVector2(1, 1));
		m_spriteSheet = new SpriteSheet(spriteSheetTexture, spriteSheetLayout.x, spriteSheetLayout.y);
	}
	else
	{
		m_spriteSheet = setSpriteSheet;
		m_usingDefaultSpriteSheet = true;
	}
	GUARANTEE_OR_DIE(m_spriteSheet != nullptr, "Animation sets must specify a sprite sheet");

	std::string frames = ParseXmlAttribute(definition, "spriteIndices", "");
	m_spriteIndices	   = ParseCSVtoInts(frames);
	//char* editableFrames = CopyString(frames);
	//std::list<char*> frameList = TokenizeString(editableFrames, ",");
	//for (auto frameIter = frameList.begin(); frameIter != frameList.end(); ++frameIter)
	//{
	//	m_spriteIndices.push_back(StringToInt(*frameIter));
	//}
	//delete editableFrames;

	if (ParseXmlAttribute(definition, "isLooping", false))
	{
		m_playbackMode = SPRITE_ANIM_MODE_LOOPING;
	}

	m_isAutoOrient = ParseXmlAttribute(definition, "autoOrient", m_isAutoOrient);

	m_frameTimeFraction = m_durationSeconds / (float)m_spriteIndices.size();
}



SpriteAnimDefinition::SpriteAnimDefinition(const std::string& name, SpriteSheet* spriteSheet, float duration, bool isLooping, std::vector<int> indices)
{
	NonXMLInitialize(name, spriteSheet, duration, isLooping, indices);
}



SpriteAnimDefinition::SpriteAnimDefinition(const std::string& name, SpriteSheet* spriteSheet, float duration, bool isLooping, int startingSpriteIndex, int numberOfFrames)
{
	std::vector<int> indices;
	for (int spriteIndex = startingSpriteIndex; spriteIndex < numberOfFrames; ++spriteIndex)
	{
		indices.push_back(spriteIndex);
	}

	NonXMLInitialize(name, spriteSheet, duration, isLooping, indices);
}



void SpriteAnimDefinition::NonXMLInitialize(const std::string& name, SpriteSheet* spriteSheet, float duration, bool isLooping, std::vector<int> indices)
{
	m_name = name;
	m_durationSeconds = duration;

	m_spriteSheet = spriteSheet;
	m_usingDefaultSpriteSheet = true;
	GUARANTEE_OR_DIE(m_spriteSheet != nullptr, "Animation sets must specify a sprite sheet");

	m_spriteIndices = indices;

	if (isLooping)
	{
		m_playbackMode = SPRITE_ANIM_MODE_LOOPING;
	}

	m_frameTimeFraction = m_durationSeconds / (float)m_spriteIndices.size();
};



SpriteAnimDefinition::~SpriteAnimDefinition()
{
	if (!m_usingDefaultSpriteSheet)
	{
		delete m_spriteSheet;
	}
}



const Texture* SpriteAnimDefinition::GetTexture() const
{
	return m_spriteSheet->GetTexture();
}



AABB2D SpriteAnimDefinition::GetTexCoordsForFractionCompleted(float fraction) const
{
	fraction = ClampFloatZeroToOne(fraction);
	int i = Interpolate(0, (int)m_spriteIndices.size() - 1, fraction);
	AABB2D textureCoordinates = m_spriteSheet->GetTexCoordsForSpriteIndex(m_spriteIndices[i]);
	return textureCoordinates;
}



const std::string& SpriteAnimDefinition::GetName() const
{
	return m_name;
}



float SpriteAnimDefinition::GetFrameTimeFraction() const
{
	return m_frameTimeFraction;
}



float SpriteAnimDefinition::GetDurationSeconds() const
{
	return m_durationSeconds;
}



SpriteAnimMode SpriteAnimDefinition::GetPlaybackMode() const
{
	return m_playbackMode;
}



bool SpriteAnimDefinition::IsAutoOrient() const
{
	return m_isAutoOrient;
}
