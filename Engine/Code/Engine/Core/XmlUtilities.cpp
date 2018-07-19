#include "Engine/Core/XmlUtilities.hpp"

//#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB2D.hpp"

//using tinyxml2::XMLElement;

bool DoesXMLAttributeExist( const XMLElement& element, const char* attributeName )
{
	bool exists = true;

	const char* value = element.Attribute(attributeName);
	if (value == nullptr)
	{
		exists = false;
	}

	return exists;
}


int ParseXmlAttribute( const XMLElement& element, const char* attributeName, int defaultValue )
{
	int value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value = StringToInt(valueString);
	}

	return value;
}



char ParseXmlAttribute( const XMLElement& element, const char* attributeName, char defaultValue )
{
	char value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value = valueString[0];
	}

	return value;
}



bool ParseXmlAttribute( const XMLElement& element, const char* attributeName, bool defaultValue )
{
	bool value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value = StringToBool(valueString);
	}

	return value;
}



float ParseXmlAttribute( const XMLElement& element, const char* attributeName, float defaultValue )
{
	float value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value = StringToFloat(valueString);
	}

	return value;
}



RGBA ParseXmlAttribute( const XMLElement& element, const char* attributeName, const RGBA& defaultValue )
{
	RGBA value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value.SetFromText(valueString);
	}

	return value;
}



Vector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Vector2& defaultValue )
{
	Vector2 value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value.SetFromText(valueString);
	}

	return value;
}



IntRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue )
{
	IntRange value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value.SetFromText(valueString);
	}

	return value;
}



FloatRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue )
{
	FloatRange value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value.SetFromText(valueString);
	}

	return value;
}



IntVector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntVector2& defaultValue )
{
	IntVector2 value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value.SetFromText(valueString);
	}

	return value;
}



std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue )
{
	std::string value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value = valueString;
	}

	return value;
}



std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue)
{
	std::string value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value = valueString;
	}

	return value;
}



AABB2D ParseXmlAttribute( const XMLElement& element, const char* attributeName, const AABB2D& defaultValue )
{
	AABB2D value = defaultValue;

	const char* valueString = element.Attribute(attributeName);
	if (valueString != nullptr)
	{
		value.SetFromText(valueString);
	}

	return value;
}
