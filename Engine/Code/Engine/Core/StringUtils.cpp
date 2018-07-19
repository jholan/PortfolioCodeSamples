#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>

#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, va_list variableArgumentList)
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}



//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//-----------------------------------------------------------------------------------------------
const char* FindFirstCharacterInString(const char* stringToSearch, const char* charsToFind)
{
	const char* first = strpbrk(stringToSearch, charsToFind);
	return first;
}


//-----------------------------------------------------------------------------------------------
char * CopyString(const std::string& string)
{
	char* editableText = new char[string.length() + 1];
	strcpy_s(editableText, string.length() + 1, string.c_str());
	return editableText;
}


//-----------------------------------------------------------------------------------------------
std::list<char*> TokenizeString(char* editableString, const char* delimiters)
{
	std::list<char*> lines;
	char* tokContext = nullptr;
	char* line = strtok_s(editableString, delimiters, &tokContext);
	while(line != nullptr)
	{
		lines.push_back(line);
		line = strtok_s(nullptr, delimiters, &tokContext);
	}
	return lines;
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> ParseTokenStringToStrings(const std::string& string, const std::string& delimiters)
{
	std::vector<std::string> substrings;

	char* editableString = CopyString(string);
	std::list<char*> rawValueList = TokenizeString(editableString, delimiters.c_str());
	for (auto listIter = rawValueList.begin(); listIter != rawValueList.end(); ++listIter)
	{
		substrings.push_back(RemoveAllWhitespace(*listIter));
	}
	delete editableString;

	return substrings;
}


//-----------------------------------------------------------------------------------------------
std::vector<int> ParseCSVtoInts(const std::string& string)
{
	std::vector<int> ints;

	char* editableString = CopyString(string);
	std::list<char*> rawValueList = TokenizeString(editableString, ",");
	for (auto listIter = rawValueList.begin(); listIter != rawValueList.end(); ++listIter)
	{
		ints.push_back(StringToInt(*listIter));
	}
	delete editableString;

	return ints;
}


//-----------------------------------------------------------------------------------------------
std::vector<float> ParseCSVtoFloats(const std::string& string)
{
	std::vector<float> floats;

	char* editableString = CopyString(string);
	std::list<char*> rawValueList = TokenizeString(editableString, ",");
	for (auto listIter = rawValueList.begin(); listIter != rawValueList.end(); ++listIter)
	{
		floats.push_back(StringToFloat(*listIter));
	}
	delete editableString;

	return floats;
}


//-----------------------------------------------------------------------------------------------
std::vector<bool> ParseCSVtoBools(const std::string& string)
{
	std::vector<bool> bools;

	char* editableString = CopyString(string);
	std::list<char*> rawValueList = TokenizeString(editableString, ",");
	for (auto listIter = rawValueList.begin(); listIter != rawValueList.end(); ++listIter)
	{
		bools.push_back(StringToBool(*listIter));
	}
	delete editableString;

	return bools;
}


//-----------------------------------------------------------------------------------------------
bool CompareStringsCaseInsensitive(const char* string1, const char* string2)
{
	bool stringsAreTheSame = true;

	const char* string1Iter = string1;
	const char* string2Iter = string2;
	while( (*string1Iter != 0) && (*string2Iter != 0) )
	{
		char lowerChar1 = (char)tolower(*string1Iter);
		char lowerChar2 = (char)tolower(*string2Iter);
		if (lowerChar1 != lowerChar2)
		{
			stringsAreTheSame = false;
			break;
		}

		++string1Iter;
		++string2Iter;
	}

	return stringsAreTheSame;
}


//-----------------------------------------------------------------------------------------------
bool CompareStrings(const char* string1, const char* string2)
{
	bool stringsAreTheSame = false;

	if (strcmp(string1, string2) == 0)
	{
		stringsAreTheSame = true;
	}

	return stringsAreTheSame;
}


//-----------------------------------------------------------------------------------------------
bool StringToBool(const char* string)
{
	bool value = true;

	const char* firstCharacter = FindFirstCharacterInString(string, "tTfF");
	if (CompareStrings(firstCharacter, "true"))
	{
		value = true;
	}
	else if (CompareStrings(firstCharacter, "false"))
	{
		value = false;
	}

	return value;
}



std::string RemoveAllWhitespace(const std::string& string)
{
	std::string spacelessString;
	spacelessString.reserve(string.size());

	for (size_t charIndex = 0; charIndex < string.size(); ++charIndex)
	{
		if (string[charIndex] != ' ')
		{
			spacelessString.push_back(string[charIndex]);
		}
	}

	return spacelessString;
}



std::string TrimWhitespaceFromFront(const std::string& string)
{
	std::string trimmedString = string;

	size_t start = trimmedString.find_first_not_of(" ");
	if (start != 0)
	{
		trimmedString.erase(0, start);
	}

	return trimmedString;
}



std::string ToLower(const std::string& string)
{
	std::string lower;
	lower.reserve(string.size());

	for (size_t charIndex = 0; charIndex < string.size(); ++charIndex)
	{
		lower.push_back((char)tolower(string[charIndex]));
	}

	return lower;
}
