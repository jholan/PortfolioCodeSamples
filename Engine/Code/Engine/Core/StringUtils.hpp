#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <list>
#include <vector>


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const char* format, va_list variableArgumentList);
const std::string Stringf( const int maxLength, const char* format, ... );


const char* FindFirstCharacterInString(const char* stringToSearch, const char* charsToFind);

char * CopyString(const std::string& string);

std::list<char*> TokenizeString(char* editableString, const char* delimiters);
std::vector<std::string> ParseTokenStringToStrings(const std::string& string, const std::string& delimiters);
std::vector<int> ParseCSVtoInts(const std::string& string);
std::vector<float> ParseCSVtoFloats(const std::string& string);
std::vector<bool> ParseCSVtoBools(const std::string& string);

bool CompareStringsCaseInsensitive(const char* string1, const char* string2);
bool CompareStrings(const char* string1, const char* string2);

bool StringToBool(const char* string);

std::string RemoveAllWhitespace(const std::string& string);
std::string TrimWhitespaceFromFront(const std::string& string);
std::string ToLower(const std::string& string);
