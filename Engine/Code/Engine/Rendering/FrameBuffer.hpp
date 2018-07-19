#pragma once

#include "Engine/Math/IntVector2.hpp"

#include "Engine/Core/Image.hpp"

class Texture;


class FrameBuffer
{
public:
	FrameBuffer(); 
	~FrameBuffer();

	void		 SetColorTarget( Texture* colorTarget); 
	void		 SetDepthStencilTarget( Texture* depthTarget); 

	// TODO: Make sure this only does work if the targets
	// have changed.
	void		 Finalize(); // Bind targets to framebuffer

	unsigned int GetHandle() const;
	IntVector2	 GetDimensions() const;

	Image		 CopyToImage() const;

public:
	unsigned int m_handle			= 0; 
	Texture* m_colorTarget			= nullptr; 
	Texture* m_depthStencilTarget	= nullptr; 
};