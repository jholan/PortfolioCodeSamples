#include "Engine/Rendering/VertexLayout.hpp"



VertexLayout::VertexLayout( size_t stride, const VertexAttribute* layout, CopyVerticies_Callback vertexCopyCallback) 
	: m_stride(stride)
	, m_callback(vertexCopyCallback)
{
	int i = 0;
	while(layout[i] != VertexAttribute::NULL_ATTRIBUTE)
	{
		m_attributes.push_back(layout[i]);

		++i;
	}
}



int VertexLayout::GetStride() const
{
	return m_stride;
}



int VertexLayout::GetAttributeCount() const
{
	return (int)m_attributes.size();
}



const VertexAttribute* VertexLayout::GetAttribute(int index) const
{
	return &(m_attributes[(size_t)index]);
}



void VertexLayout::CopyVerticies(void* dst, const VertexBuilder* src, unsigned int count) const
{
	m_callback(dst, src, count);
}