#include "Game/Components/Tags.hpp"

#include <list>

#include "Engine/Core/StringUtils.hpp"



Tags::Tags()
{

}



Tags::~Tags()
{

}



int Tags::Count() const
{
	return (int)m_tags.size();
}



void Tags::SetOrRemoveCSV(const std::string& commaSeparatedTagNames)
{
	std::string noSpaces = RemoveAllWhitespace(commaSeparatedTagNames);
	std::string lower = ToLower(noSpaces);
	char* editableSafeString = CopyString(lower);
	std::list<char*> tagStrings = TokenizeString(editableSafeString, ",");

	for (auto iter = tagStrings.begin(); iter != tagStrings.end(); ++iter)
	{
		if ((*iter)[0] == '!')
		{
			Remove((*iter) + 1);
		}
		else
		{
			Set(*iter);
		}
	}

	delete editableSafeString;
}



bool Tags::HasCSV(const std::string& commaSeparatedTagNames)
{
	bool allPresent = true;

	std::string noSpaces = RemoveAllWhitespace(commaSeparatedTagNames);
	//std::string lower = ToLower(noSpaces);
	//char* editableSafeString = CopyString(lower);
	char* editableSafeString = CopyString(noSpaces);
	std::list<char*> tagStrings = TokenizeString(editableSafeString, ",");

	for (auto iter = tagStrings.begin(); iter != tagStrings.end(); ++iter)
	{
		if ((*iter)[0] == '!')
		{
			if (Has((*iter) + 1) != false)
			{
				allPresent = false;
				break;
			}
		}
		else
		{
			if (Has(*iter) != true)
			{
				allPresent = false;
				break;
			}
		}
	}

	delete editableSafeString;
	return allPresent;
}



void Tags::Set(const std::string& tagName)
{
	m_tags.insert(tagName);
}



void Tags::Remove(const std::string& tagName)
{
	auto locationInSet = m_tags.find(tagName);
	if (locationInSet != m_tags.end())
	{
		m_tags.erase(locationInSet);
	}
}



bool Tags::Has(const std::string& tagName)
{
	bool present = true;

	auto locationInSet = m_tags.find(tagName);
	if (locationInSet == m_tags.end())
	{
		present = false;
	}

	return present;
}
