#include "Engine/Rendering/RenderBuffer.h"

#include "Engine/Rendering/GLFunctions.h"


RenderBuffer::~RenderBuffer()
{
	// cleanup for a buffer; 
	if (m_arrayBufferHandle != NULL) {
		glDeleteBuffers( 1, &(GLuint)m_arrayBufferHandle ); 
		m_arrayBufferHandle = NULL; 
	}
}


bool RenderBuffer::CopyToGPU( size_t const byte_count, void const *data ) 
{
	// handle is a GLuint member - used by OpenGL to identify this buffer
	// if we don't have one, make one when we first need it [lazy instantiation]
	if (m_arrayBufferHandle == NULL) {
		glGenBuffers( 1, &m_arrayBufferHandle ); 
	}

	// Bind the buffer to a slot, and copy memory
	// GL_DYNAMIC_DRAW means the memory is likely going to change a lot (we'll get
	// during the second project)
	glBindBuffer( GL_UNIFORM_BUFFER, m_arrayBufferHandle ); 
	glBufferData( GL_UNIFORM_BUFFER, byte_count, data, GL_DYNAMIC_DRAW ); 

	// buffer_size is a size_t member variable I keep around for 
	// convenience
	m_arrayBufferSize = byte_count; 
	return true; 
}



unsigned int RenderBuffer::GetArrayBufferHandle() const
{
	return m_arrayBufferHandle;
}


size_t RenderBuffer::GetSize() const
{
	return m_arrayBufferSize;
}

