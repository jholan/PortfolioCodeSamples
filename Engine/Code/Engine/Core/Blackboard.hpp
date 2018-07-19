#pragma once

#include <string>
#include <map>

#include "Engine/Core/XmlUtilities.hpp"

class RGBA;
class Vector2;
class IntVector2;
class FloatRange;
class IntRange;

class Blackboard
{
public:
	// Composition
	Blackboard() {};  // Does Nothing.
	~Blackboard() {}; // Does Nothing.
	void			PopulateFromXmlElementAttributes( const XMLElement& element );


	void			SetValue( const std::string& keyName, const std::string& newValue );

	bool			GetValue( const std::string& keyName, bool defaultValue ) const;
	int			    GetValue( const std::string& keyName, int defaultValue ) const;
	float			GetValue( const std::string& keyName, float defaultValue ) const;
	std::string		GetValue( const std::string& keyName, std::string defaultValue ) const;
	std::string		GetValue( const std::string& keyName, const char* defaultValue ) const;
	RGBA			GetValue( const std::string& keyName, const RGBA& defaultValue ) const;
	Vector2			GetValue( const std::string& keyName, const Vector2& defaultValue ) const;
	IntVector2		GetValue( const std::string& keyName, const IntVector2& defaultValue ) const;
	FloatRange		GetValue( const std::string& keyName, const FloatRange& defaultValue ) const;
	IntRange		GetValue( const std::string& keyName, const IntRange& defaultValue ) const;


private:	
	std::map< std::string, std::string >	m_keyValuePairs;
};