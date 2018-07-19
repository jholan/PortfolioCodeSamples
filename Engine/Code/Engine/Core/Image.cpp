#include "Engine/Core/Image.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "ThirdParty/stb/stb_image.h"

#pragma warning( disable : 4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image_write.h"
#pragma warning( default : 4996)


//------------------------------------------------------------------------
Image::Image(const std::string& imageFilePath, bool flipY)
{
	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk, and create an OpenGL texture instance from it
	stbi_set_flip_vertically_on_load(flipY);
	unsigned char* imageData = stbi_load( imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );

	int numTexels = m_dimensions.x * m_dimensions.y;
	m_texels.reserve(numTexels);
	if (numComponents == 3)
	{
		for (int componentIndex = 0; componentIndex < numTexels * numComponents; componentIndex += 3)
		{
			RGBA texel = RGBA(imageData[componentIndex], imageData[componentIndex + 1], imageData[componentIndex + 2]);
			m_texels.push_back(texel);
		}
	}
	if (numComponents == 4)
	{
		for (int componentIndex = 0; componentIndex < numTexels * numComponents; componentIndex += 4)
		{
			RGBA texel = RGBA(imageData[componentIndex], imageData[componentIndex + 1], imageData[componentIndex + 2], imageData[componentIndex + 3]);
			m_texels.push_back(texel);
		}
	}

	stbi_image_free( imageData );
}


//------------------------------------------------------------------------
Image::Image(const Image& otherImage)
{
	m_dimensions = otherImage.m_dimensions;
	m_texels = otherImage.m_texels;
}


//------------------------------------------------------------------------
void Image::operator=(const Image& otherImage)
{
	m_dimensions = otherImage.m_dimensions;
	m_texels = otherImage.m_texels;
}


//------------------------------------------------------------------------
RGBA Image::GetTexel(int x, int y) const
{
	int texelIndex = x + (y * m_dimensions.x);
	RGBA texel = m_texels[texelIndex];
	return texel;
}


//------------------------------------------------------------------------
void Image::SetTexel(int x, int y, const RGBA& color)
{
	int texelIndex = x + (y * m_dimensions.x);
	m_texels[texelIndex] = color;
}


//------------------------------------------------------------------------
IntVector2 Image::GetDimensions() const
{
	return m_dimensions;
}


//------------------------------------------------------------------------
const unsigned char* Image::GetTexelDataAsUChars() const
{
	return (unsigned char*)m_texels.data();
}



void Image::SetTexelDataFromUChars(const unsigned char* data, int width, int height, int numberOfComponents)
{
	m_dimensions.x = width;
	m_dimensions.y = height;

	unsigned int numPixels = width * height;
	for (unsigned int i = 0; i < numPixels * numberOfComponents; i += numberOfComponents)
	{
		RGBA texel = RGBA();
		
		if (numberOfComponents == 3 || numberOfComponents == 4)
		{
			texel.r = data[i + 0];
			texel.g = data[i + 1];
			texel.b = data[i + 2];

			if (numberOfComponents == 4)
			{
				texel.a = data[i + 3];
			}
		}

		m_texels.push_back(texel);
	}
}


#include <windows.h>

bool DoesFolderExist(const std::string& foldername)
{
	DWORD ftyp = GetFileAttributesA(foldername.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}



void CreateFolder(const std::string& foldername)
{
	CreateDirectoryA( foldername.c_str(), NULL );
}


std::string GetFolderPath(const std::string& filename)
{
	std::string folderpath = "";

	std::size_t found = filename.find_last_of("/\\");
	if (found != std::string::npos)
	{
		folderpath = filename.substr(0,found);
	}

	return folderpath;
}


std::string GetCurrentTimestamp()
{
	SYSTEMTIME currentSystemTime;
	GetSystemTime( &currentSystemTime );

	std::string timestamp = Stringf("_%04hu%02hu%02hu_%02hu%02hu%02hu", currentSystemTime.wYear, currentSystemTime.wMonth, currentSystemTime.wDay, currentSystemTime.wHour, currentSystemTime.wMinute, currentSystemTime.wSecond);
	return timestamp;
}


std::string AppendTimestamp(const std::string& filename)
{
	std::string timestampedFile = filename;
	size_t extensionStart = filename.find_last_of('.');
	if (extensionStart != std::string::npos)
	{
		timestampedFile = filename.substr(0,extensionStart) + GetCurrentTimestamp() + filename.substr(extensionStart, std::string::npos);
	}
	return timestampedFile;
}


//------------------------------------------------------------------------
void Image::WriteToFile( const std::string& filename, bool appendTimestamp) const
{
	if (!DoesFolderExist(GetFolderPath(filename)))
	{
		CreateFolder(GetFolderPath(filename));
	}

	std::string fullFilename = filename;
	if (appendTimestamp)
	{
		fullFilename = AppendTimestamp(filename);
	}

	stbi_flip_vertically_on_write(1);
	stbi_write_png(fullFilename.c_str(), m_dimensions.x, m_dimensions.y, 4, GetTexelDataAsUChars(), 4 * m_dimensions.x);
}

