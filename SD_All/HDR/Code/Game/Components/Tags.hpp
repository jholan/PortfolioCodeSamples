#pragma once

#include <string>
#include <unordered_set>

#include "Engine/Core/XmlUtilities.hpp"



// NOTE to Add/ check if a tag exists use tag_name ie: "blessed"
//		to Remove/ check if a tag is missing use !tag_name ie: "!blessed"
class Tags
{
public:
	Tags();
	~Tags();

	int Count() const;

	void SetOrRemoveCSV(const std::string& commaSeparatedTagNames); // "blessed,!poisoned"
	bool HasCSV(const std::string& commaSeparatedTagNames); // "good,!cursed"

	void Set(const std::string& tagName);
	void Remove(const std::string& tagName);
	bool Has(const std::string& tagName);


private:
	std::unordered_set<std::string> m_tags;
};
