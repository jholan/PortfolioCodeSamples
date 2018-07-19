#pragma once



class RenderBuffer
{
public:
	RenderBuffer() {};
	virtual ~RenderBuffer();

	bool CopyToGPU( size_t const byte_count, void const *data );

	unsigned int GetArrayBufferHandle() const;
	size_t		 GetSize() const;				// Bytes



private:
	unsigned int	m_arrayBufferHandle = 0;
	size_t			m_arrayBufferSize = 0;

};