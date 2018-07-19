#pragma once

#include <string>
#include <vector>

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/IntVector2.hpp"



class Image
{
public:
	Image() {};
	explicit Image( const std::string& imageFilePath, bool flipY = true );
	Image( const Image& otherImage );
	void operator=( const Image& otherImage );
	
	RGBA GetTexel( int x, int y ) const; 				// (0,0) is top-left
	void SetTexel( int x, int y, const RGBA& color );

	IntVector2 GetDimensions() const;

	const unsigned char* GetTexelDataAsUChars() const;
	void SetTexelDataFromUChars(const unsigned char* data, int width, int height, int numberOfComponents);

	void WriteToFile( const std::string& filename, bool appendTimestamp = true ) const;


private:
	IntVector2			m_dimensions;
	std::vector<RGBA>	m_texels; // ordered left-to-right, then down, from [0]=(0,0) at top-left

};