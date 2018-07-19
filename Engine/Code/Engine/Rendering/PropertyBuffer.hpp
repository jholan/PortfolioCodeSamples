#pragma once

#include <string>

#include "Engine/Rendering/UniformBuffer.hpp"

class PropertyBlockDescription;



class PropertyBlock
{
public:
	PropertyBlock();
	PropertyBlock(const PropertyBlock& toCopy);

	const PropertyBlockDescription*		GetDescription() const;
	const std::string&					GetName() const;

	void								SetSize(unsigned int sizeInBytes);
	void								SetDescription(const PropertyBlockDescription* description);

	void								RawSet(unsigned int offsetIntoBufferInBytes, const void* data, unsigned int dataSize);

	void								Bind();


private:
	const PropertyBlockDescription*		m_description;
	unsigned int						m_bindPoint;
	UniformBlock						m_uniformBlock;
};