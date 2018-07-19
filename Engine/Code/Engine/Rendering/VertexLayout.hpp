#pragma once

#include <vector>

#include "Engine/Rendering/VertexAttribute.hpp"

struct VertexBuilder;

typedef void (*CopyVerticies_Callback)( void* dst, const VertexBuilder* src, unsigned int count ); 


class VertexLayout
{
public:
	VertexLayout(size_t stride, const VertexAttribute* layout, CopyVerticies_Callback vertexCopyCallback);

	int GetStride() const;

	int GetAttributeCount() const;
	const VertexAttribute* GetAttribute(int index) const;

	void CopyVerticies(void* dst, const VertexBuilder* src, unsigned int count) const;


private:
	std::vector<VertexAttribute> m_attributes; 
	int							 m_stride;     // how far between element

	CopyVerticies_Callback		 m_callback; // Generate vertex buffer from master VertexBuilder buffer
};