#include "Engine/Rendering/FrameBuffer.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Rendering/GLFunctions.h"
#include "Engine/Rendering/Texture.hpp"

#define _DEBUG 1



FrameBuffer::FrameBuffer()
{
	glGenFramebuffers( 1, &m_handle ); 
}



FrameBuffer::~FrameBuffer()
{
	if (m_handle != NULL)
	{
		glDeleteFramebuffers( 1, &m_handle ); 
		m_handle = NULL; 
	}
}



void FrameBuffer::SetColorTarget( Texture* colorTarget )
{
	m_colorTarget = colorTarget;
}



void FrameBuffer::SetDepthStencilTarget( Texture* depthTarget )
{
	m_depthStencilTarget = depthTarget;
}



void FrameBuffer::Finalize()
{
	// NOTE: GL_FRAMEBUFFER == GL_DRAW_FRAMEBUFFER
	glBindFramebuffer( GL_FRAMEBUFFER, m_handle ); 

	if(m_colorTarget != nullptr)
	{
		// keep track of which outputs go to which attachments; 
		GLenum targets[1]; 

		// Bind a color target to an attachment point
		// and keep track of which locations to to which attachments. 
		glFramebufferTexture(	GL_FRAMEBUFFER, 
								GL_COLOR_ATTACHMENT0 + 0, 
								m_colorTarget->GetHandle(), 0); 
	
		// 0 to to attachment 0
		targets[0] = GL_COLOR_ATTACHMENT0 + 0; 

		// Update target bindings
		glDrawBuffers( 1, targets ); 
	}
	else
	{
		GLenum targets[1];
		targets[0] = GL_NONE; // Tell gl we wont be writing out any colors

		glFramebufferTexture(	GL_FRAMEBUFFER, 
			GL_COLOR_ATTACHMENT0 + 0, 
			0, 0); 

		glDrawBuffers(1, targets);
	}

	// Bind depth if available;
	if (m_depthStencilTarget == nullptr) 
	{
		glFramebufferTexture(	GL_FRAMEBUFFER, 
								GL_DEPTH_STENCIL_ATTACHMENT, 
								NULL, 0); 
	} 
	else 
	{
		glFramebufferTexture(	GL_FRAMEBUFFER, 
								GL_DEPTH_STENCIL_ATTACHMENT, 
								m_depthStencilTarget->GetHandle(), 0); 
	}

	// Error Check - recommend only doing in debug
#if defined(_DEBUG)
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if (status != GL_FRAMEBUFFER_COMPLETE) 
	{
		DebuggerPrintf( "Failed to create framebuffer:  %u", status );
		return;
	}
#endif

	return;
}



unsigned int FrameBuffer::GetHandle() const
{
	return m_handle;
}



IntVector2 FrameBuffer::GetDimensions() const
{
	IntVector2 dimensions = IntVector2(0, 0);

	if (m_colorTarget != nullptr)
	{
		dimensions = m_colorTarget->GetDimensions();
	}
	else if (m_depthStencilTarget != nullptr)
	{
		dimensions = m_depthStencilTarget->GetDimensions();
	}

	return dimensions;
}



Image FrameBuffer::CopyToImage() const
{
	Image copy = m_colorTarget->CopyToImage();
	return copy;
}