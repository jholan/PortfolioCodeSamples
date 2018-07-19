#pragma once
#include <string>

#include "ThirdParty/TinyXML2/tinyxml2.h"
using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using tinyxml2::XMLAttribute;

class RGBA;
class Vector2;
class IntVector2;
class IntRange;
class FloatRange;
class AABB2D;



bool DoesXMLAttributeExist( const XMLElement& element, const char* attributeName );

int ParseXmlAttribute( const XMLElement& element, const char* attributeName, int defaultValue );
char ParseXmlAttribute( const XMLElement& element, const char* attributeName, char defaultValue );
bool ParseXmlAttribute( const XMLElement& element, const char* attributeName, bool defaultValue );
float ParseXmlAttribute( const XMLElement& element, const char* attributeName, float defaultValue );
RGBA ParseXmlAttribute( const XMLElement& element, const char* attributeName, const RGBA& defaultValue );
Vector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Vector2& defaultValue );
IntRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue );
FloatRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue );
IntVector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntVector2& defaultValue );
std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue );
std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue=nullptr );
AABB2D ParseXmlAttribute( const XMLElement& element, const char* attributeName, const AABB2D& defaultValue );
