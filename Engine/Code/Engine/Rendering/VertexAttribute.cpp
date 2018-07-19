#include "Engine/Rendering/VertexAttribute.hpp"




VertexAttribute::VertexAttribute( const std::string& name, unsigned int type, unsigned int elementCount, bool normalize, size_t offset ) 
	: m_name(name)
	, m_glType(type)
	, m_elementCount(elementCount)
	, m_isNormalized(normalize)
	, m_memberOffset(offset)
{
}



bool VertexAttribute::operator==(const VertexAttribute& rhs) const
{
	bool isEqual = true;

	if (m_name != rhs.m_name)
	{
		isEqual = false;
	}
	if (m_glType != rhs.m_glType)
	{
		isEqual = false;
	}
	if (m_elementCount != rhs.m_elementCount)
	{
		isEqual = false;
	}
	if (m_isNormalized != rhs.m_isNormalized)
	{
		isEqual = false;
	}
	if (m_memberOffset != rhs.m_memberOffset)
	{
		isEqual = false;
	}

	return isEqual;
}



bool VertexAttribute::operator!=(const VertexAttribute& rhs) const
{
	return !(*this == rhs);
}



const std::string& VertexAttribute::GetName() const
{
	return m_name;
}



unsigned int VertexAttribute::GetGLType() const
{
	return m_glType;
}



int VertexAttribute::GetElementCount() const
{
	return m_elementCount;
}



int	VertexAttribute::GetMemberOffset() const
{
	return m_memberOffset;
}



bool VertexAttribute::ShouldNormalize() const
{
	return m_isNormalized;
}



const VertexAttribute VertexAttribute::NULL_ATTRIBUTE = VertexAttribute();