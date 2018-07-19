#pragma once
#include "Engine/Rendering/RenderBuffer.h"



class UniformBlock
{
public:
	UniformBlock() {};
	//UniformBuffer(UniformBuffer&& old);
	~UniformBlock();

	void BindTo(unsigned int bindPoint);

	void SetSize(unsigned int sizeInBytes);
	unsigned int GetSize() const { return m_cpuDataSize; };

	template <typename T>
	void Set(const T& data)
	{
		ClearCPUData();
		m_cpuData = new T(data);
		m_cpuDataSize = sizeof(T);
	}

	template <typename T>
	T* As()
	{
		if (m_cpuDataSize < sizeof(T))
		{
			Set<T>(T());
		}
		m_isDirty = true;

		return (T*)m_cpuData; 
	}

	template <typename T>
	const T* As() const
	{
		return (const T*)m_cpuData; 
	}

	void ClearCPUData();


	template <typename T>
	static UniformBlock For(const T& data) 
	{
		UniformBlock ubo = UniformBlock();

		ubo.Set<T>(data);

		return ubo;
	}

private:
	// CPU Side
	bool  m_isDirty = true;
	void* m_cpuData = nullptr;
	int   m_cpuDataSize = 0;

	// GPU Side
	RenderBuffer m_renderBuffer;
};