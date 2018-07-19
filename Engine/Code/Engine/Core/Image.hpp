#pragma once

#include <string>
#include <vector>

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/IntVector2.hpp"

enum eImageType
{
	IMAGE_TYPE_INVALID = -1,

	IMAGE_TYPE_BYTE = 0,
	IMAGE_TYPE_FLOAT,

	IMAGE_TYPE_COUNT
};


class Image
{
public:
	Image() {};
	explicit Image( const std::string& imageFilePath, bool flipY = true );
	Image( const Image& otherImage );
	void operator=( const Image& otherImage );
	
	RGBA					GetTexel_RGBA( int x, int y ) const; 				// (0,0) is top-left
	Vector4					GetTexel_Vector4(int x, int y) const;

	void					SetTexel_RGBA( int x, int y, const RGBA& color );
	void					SetTexel_Vector4(int x, int y, const Vector4& color);


	IntVector2				GetDimensions() const;


	const void*				GetTexelData() const;
	const unsigned char*	GetTexelDataAsUChars() const;
	const float*			GetTexelDataAsFloats() const;

	void					SetTexelDataFromUChars(const unsigned char* data, int width, int height, int numberOfComponents);
	void					SetTexelDataFromFloats(const float*			data, int width, int height, int numberOfComponents);

	void					WriteToFile( const std::string& filename, bool appendTimestamp = true ) const;

	void					SetImageType(eImageType imageType);
	eImageType				GetImageType() const;


private:
	void					LoadByteData();
	void					LoadFloatData();

	std::string				m_filePath;
	eImageType				m_imageType = IMAGE_TYPE_BYTE;
	bool					m_isFlipped = false;
	

	IntVector2				m_dimensions;
	std::vector<RGBA>		m_texels_Byte; // ordered left-to-right, then down, from [0]=(0,0) at top-left
	std::vector<Vector4>	m_texels_Float;
};