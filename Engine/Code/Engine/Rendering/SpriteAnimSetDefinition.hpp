#pragma once

#include <string>
#include <map>
#include <vector>

#include "Engine/Core/XmlUtilities.hpp"

class Renderer;
class SpriteSheet;
class SpriteAnimDefinition;
class SpriteAnim;


class SpriteAnimSetDefinition
{
public:
	SpriteAnimSetDefinition(const XMLElement& definition);
	~SpriteAnimSetDefinition();

	std::vector<SpriteAnim*> GetSpriteAnims() const;

private:
	SpriteSheet* m_defaultSpriteSheet = nullptr;
	std::map<std::string, SpriteAnimDefinition*> m_animDefinitionsByName;
};