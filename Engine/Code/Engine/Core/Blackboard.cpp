#include "Engine/Core/Blackboard.hpp"

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"



void Blackboard::PopulateFromXmlElementAttributes( const XMLElement& element )
{
	const XMLAttribute* attribute = element.FirstAttribute();
	while(attribute != nullptr)
	{
		SetValue(attribute->Name(), attribute->Value());
		attribute = attribute->Next();
	}
}


void Blackboard::SetValue( const std::string& keyName, const std::string& newValue )
{
	m_keyValuePairs[keyName] = newValue;
}



bool Blackboard::GetValue( const std::string& keyName, bool defaultValue ) const
{
	bool value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value = StringToBool(it->second.c_str());
	}

	return value;
}



int Blackboard::GetValue( const std::string& keyName, int defaultValue ) const
{
	int value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value = StringToInt(it->second.c_str());
	}

	return value;
}



float Blackboard::GetValue( const std::string& keyName, float defaultValue ) const
{
	float value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value = StringToFloat(it->second.c_str());
	}

	return value;
}



std::string Blackboard::GetValue( const std::string& keyName, std::string defaultValue ) const
{
	std::string value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value = it->second;
	}

	return value;
}



std::string Blackboard::GetValue( const std::string& keyName, const char* defaultValue ) const
{
	std::string value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value = it->second;
	}

	return value;
}



RGBA Blackboard::GetValue( const std::string& keyName, const RGBA& defaultValue ) const
{
	RGBA value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value.SetFromText(it->second.c_str());
	}

	return value;
}



Vector2 Blackboard::GetValue( const std::string& keyName, const Vector2& defaultValue ) const
{
	Vector2 value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value.SetFromText((it->second).c_str());
	}

	return value;
}



IntVector2 Blackboard::GetValue( const std::string& keyName, const IntVector2& defaultValue ) const
{
	IntVector2 value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value.SetFromText(it->second.c_str());
	}

	return value;
}



FloatRange Blackboard::GetValue( const std::string& keyName, const FloatRange& defaultValue ) const
{
	FloatRange value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value.SetFromText(it->second.c_str());
	}

	return value;
}



IntRange Blackboard::GetValue( const std::string& keyName, const IntRange& defaultValue ) const
{
	IntRange value = defaultValue;

	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		value.SetFromText(it->second.c_str());
	}

	return value;
}
