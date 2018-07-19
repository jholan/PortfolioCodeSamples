#include "Engine/Rendering/PropertyBuffer.hpp"

#include <cstring>

#include "Engine/Rendering/UniformBuffer.hpp"
#include "Engine/Rendering/ShaderUniformDescriptions.hpp"



PropertyBlock::PropertyBlock()
{
	m_description = nullptr;
	m_bindPoint = 100;
}



PropertyBlock::PropertyBlock(const PropertyBlock& toCopy)
{
	SetDescription(toCopy.GetDescription());
	SetSize(toCopy.m_uniformBlock.GetSize());
	RawSet(0, (void*)toCopy.m_uniformBlock.As<unsigned char>(), toCopy.m_uniformBlock.GetSize());
}



const PropertyBlockDescription* PropertyBlock::GetDescription() const
{
	return m_description;
}



const std::string& PropertyBlock::GetName() const
{
	return m_description->GetName();
}



void PropertyBlock::SetSize(unsigned int sizeInBytes)
{
	m_uniformBlock.SetSize(sizeInBytes);
}



void PropertyBlock::SetDescription(const PropertyBlockDescription* description)
{
	m_description = description;
	m_bindPoint = m_description->GetBindPoint();
}



void PropertyBlock::RawSet(unsigned int offsetIntoBufferInBytes, const void* data, unsigned int dataSize)
{
	unsigned char* rawMemory = m_uniformBlock.As<unsigned char>();
	memcpy((void*)(rawMemory + offsetIntoBufferInBytes), data, dataSize);
}



void PropertyBlock::Bind()
{
	m_uniformBlock.BindTo(m_bindPoint);
}