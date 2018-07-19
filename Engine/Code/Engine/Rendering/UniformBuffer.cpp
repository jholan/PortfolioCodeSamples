#include "Engine/Rendering/UniformBuffer.hpp"

#include <cstdlib>
#include <cstring>

#include "Engine/Rendering/GLFunctions.h"



/*UniformBuffer::UniformBuffer(UniformBuffer&& old)
{
	m_isDirty = old.m_isDirty;
	m_cpuData = old.m_cpuData;
	m_cpuDataSize = old.m_cpuDataSize;

	old.m_isDirty = true;
	old.m_cpuData = nullptr;
	old.m_cpuDataSize = 0;
}*/



UniformBlock::~UniformBlock()
{
	if (m_cpuData != nullptr)
	{
		delete m_cpuData;
		m_cpuData = nullptr;
		m_cpuDataSize = 0;
	}

	m_isDirty = true;
}



void UniformBlock::BindTo(unsigned int bindPoint)
{
	if (m_isDirty)
	{
		m_isDirty = false;
		m_renderBuffer.CopyToGPU(m_cpuDataSize, m_cpuData);
	}
	glBindBufferBase( GL_UNIFORM_BUFFER, bindPoint, m_renderBuffer.GetArrayBufferHandle() ); 
}



void UniformBlock::SetSize(unsigned int sizeInBytes)
{
	// Short Circuit
	if (m_cpuDataSize == (int)sizeInBytes)
	{
		return;
	}
	
	ClearCPUData();
	m_cpuData = malloc(sizeInBytes);
	memset(m_cpuData, NULL, sizeInBytes);
	m_cpuDataSize = sizeInBytes;
}



void UniformBlock::ClearCPUData()
{
	m_isDirty = true;
	if (m_cpuData != nullptr)
	{
		delete m_cpuData;
		m_cpuData = nullptr;
	}
	m_cpuDataSize = 0;
}