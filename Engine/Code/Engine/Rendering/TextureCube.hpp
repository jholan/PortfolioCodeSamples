#pragma once

#include <string>

#include "Engine/Math/IntVector2.hpp"

#include "Engine/Core/Image.hpp"



class TextureCube
{
public:
	bool		 IsValid() const;
	unsigned int GetHandle() const;
	IntVector2	 GetSideDimensions() const;

	TextureCube();
	TextureCube(const Image& image);
	TextureCube(const std::string& imageFilepath);
	TextureCube(const Image** images);

private:
	bool			m_isValid = true;
	unsigned int	m_textureID;
	IntVector2		m_dimensions;
};