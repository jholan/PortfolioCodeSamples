#include "Engine/Core/Image.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "ThirdParty/stb/stb_image.h"

#pragma warning( disable : 4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image_write.h"
#pragma warning( default : 4996)



const char* byteFileExtensions[] = 
{
	"png"
};

const char* floatFileExtensions[] = 
{
	"hdr"
};

//------------------------------------------------------------------------
eImageType DetermineTypeFromFilepath(const std::string& filepath)
{
	eImageType type = IMAGE_TYPE_INVALID;
	size_t fileExtensionStart = filepath.find_last_of('.');


	GUARANTEE_OR_DIE(fileExtensionStart != std::string::npos, Stringf("Image.cpp DetermineTypeFromFilepath Filepath = %s has no file extensions", filepath.c_str()).c_str());


	// Determine the type
	std::string fileExtension = ToLower(filepath.substr(fileExtensionStart + 1));

	// Parse for LDR types
	int numByteFileExtensions = sizeof(byteFileExtensions) / sizeof(byteFileExtensions[0]);
	for (int i = 0; i < numByteFileExtensions; ++i)
	{
		if (fileExtension == byteFileExtensions[i])
		{
			type = IMAGE_TYPE_BYTE;
			break;
		}
	}

	// Parse for HDR Types
	if (type == IMAGE_TYPE_INVALID)
	{
		int numFloatFileExtensions = sizeof(floatFileExtensions) / sizeof(floatFileExtensions[0]);
		for (int i = 0; i < numFloatFileExtensions; ++i)
		{
			if (fileExtension == floatFileExtensions[i])
			{
				type = IMAGE_TYPE_FLOAT;
				break;
			}
		}
	}

	GUARANTEE_OR_DIE(type != IMAGE_TYPE_INVALID, Stringf("Image.cpp DetermineTypeFromFilepath FileExtension = %s is not a filetype supported by this engine", fileExtension.c_str()).c_str());

	return type;
}


//------------------------------------------------------------------------
Image::Image(const std::string& imageFilePath, bool flipY)
{
	m_filePath = imageFilePath;
	m_imageType = DetermineTypeFromFilepath(imageFilePath);
	m_isFlipped = flipY;


	// Load the data
	switch(m_imageType)
	{
	case IMAGE_TYPE_BYTE:
	{
		LoadByteData();
		break;
	}
	case IMAGE_TYPE_FLOAT:
	{
		LoadFloatData();
		break;
	}
	case IMAGE_TYPE_INVALID:
	case IMAGE_TYPE_COUNT:
	default:
	{
		// Never get here
		GUARANTEE_OR_DIE(false, "Image::Image() invalid image type");
		break;
	}
	}
}


//------------------------------------------------------------------------
void Image::LoadByteData()
{
	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

									// Load (and decompress) the image RGB(A) bytes from a file on disk, and create an OpenGL texture instance from it
	stbi_set_flip_vertically_on_load(m_isFlipped);
	unsigned char* imageData = stbi_load( m_filePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );

	int numTexels = m_dimensions.x * m_dimensions.y;
	m_texels_Byte.reserve(numTexels);
	if (numComponents == 3)
	{
		for (int componentIndex = 0; componentIndex < numTexels * numComponents; componentIndex += 3)
		{
			RGBA texel = RGBA(imageData[componentIndex], imageData[componentIndex + 1], imageData[componentIndex + 2]);
			m_texels_Byte.push_back(texel);
		}
	}
	if (numComponents == 4)
	{
		for (int componentIndex = 0; componentIndex < numTexels * numComponents; componentIndex += 4)
		{
			RGBA texel = RGBA(imageData[componentIndex], imageData[componentIndex + 1], imageData[componentIndex + 2], imageData[componentIndex + 3]);
			m_texels_Byte.push_back(texel);
		}
	}

	stbi_image_free( imageData );
}


//------------------------------------------------------------------------
void Image::LoadFloatData()
{
	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk, and create an OpenGL texture instance from it
	stbi_set_flip_vertically_on_load(m_isFlipped);
	float* imageData = stbi_loadf( m_filePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );
	
	int numTexels = m_dimensions.x * m_dimensions.y;

	float maxV = 0.0f;
	for (int i = 0; i < numTexels * numComponents; ++i)
	{
		maxV = Max(maxV, imageData[i]);
	}


	m_texels_Byte.reserve(numTexels);
	if (numComponents == 3)
	{
		for (int componentIndex = 0; componentIndex < numTexels * numComponents; componentIndex += 3)
		{
			Vector4 texel = Vector4(imageData[componentIndex], imageData[componentIndex + 1], imageData[componentIndex + 2], 1.0f);
			m_texels_Float.push_back(texel);
		}
	}
	if (numComponents == 4)
	{
		for (int componentIndex = 0; componentIndex < numTexels * numComponents; componentIndex += 4)
		{
			Vector4 texel = Vector4(imageData[componentIndex], imageData[componentIndex + 1], imageData[componentIndex + 2], imageData[componentIndex + 3]);
			m_texels_Float.push_back(texel);
		}
	}

	stbi_image_free( imageData );
}


//------------------------------------------------------------------------
Image::Image(const Image& otherImage)
{
	m_filePath = otherImage.m_filePath;
	m_imageType = otherImage.m_imageType;
	m_isFlipped = otherImage.m_isFlipped;

	m_dimensions = otherImage.m_dimensions;
	m_texels_Byte = otherImage.m_texels_Byte;
	m_texels_Float = otherImage.m_texels_Float;
}


//------------------------------------------------------------------------
void Image::operator=(const Image& otherImage)
{
	m_filePath = otherImage.m_filePath;
	m_imageType = otherImage.m_imageType;
	m_isFlipped = otherImage.m_isFlipped;

	m_dimensions = otherImage.m_dimensions;
	m_texels_Byte = otherImage.m_texels_Byte;
	m_texels_Float = otherImage.m_texels_Float;
}


//------------------------------------------------------------------------
RGBA Image::GetTexel_RGBA(int x, int y) const
{
	int texelIndex = x + (y * m_dimensions.x);
	RGBA texel = m_texels_Byte[texelIndex];
	return texel;
}


//------------------------------------------------------------------------
Vector4	Image::GetTexel_Vector4(int x, int y) const
{
	int texelIndex = x + (y * m_dimensions.x);
	Vector4 texel = m_texels_Float[texelIndex];
	return texel;
}


//------------------------------------------------------------------------
void Image::SetTexel_RGBA(int x, int y, const RGBA& color)
{
	int texelIndex = x + (y * m_dimensions.x);
	m_texels_Byte[texelIndex] = color;
}


//------------------------------------------------------------------------
void Image::SetTexel_Vector4(int x, int y, const Vector4& color)
{
	int texelIndex = x + (y * m_dimensions.x);
	m_texels_Float[texelIndex] = color;
}


//------------------------------------------------------------------------
IntVector2 Image::GetDimensions() const
{
	return m_dimensions;
}


//------------------------------------------------------------------------
const void*	Image::GetTexelData() const
{
	void* data = nullptr;


	switch(m_imageType)
	{
	case IMAGE_TYPE_BYTE:
	{
		data = (void*)m_texels_Byte.data();
		break;
	}
	case IMAGE_TYPE_FLOAT:
	{
		data = (void*)m_texels_Float.data();
		break;
	}
	case IMAGE_TYPE_INVALID:
	case IMAGE_TYPE_COUNT:
	default:
	{
		GUARANTEE_OR_DIE(false, "Image::GetTexelData() invalid eImageType");
		break;
	}
	}


	return data;
}


//------------------------------------------------------------------------
const unsigned char* Image::GetTexelDataAsUChars() const
{
	return (unsigned char*)m_texels_Byte.data();
}



//------------------------------------------------------------------------
const float* Image::GetTexelDataAsFloats() const
{
	return (float*)m_texels_Float.data();
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

		m_texels_Byte.push_back(texel);
	}
}



void Image::SetTexelDataFromFloats(const float* data, int width, int height, int numberOfComponents)
{
	m_dimensions.x = width;
	m_dimensions.y = height;

	unsigned int numPixels = width * height;
	for (unsigned int i = 0; i < numPixels * numberOfComponents; i += numberOfComponents)
	{
		Vector4 texel = Vector4();

		if (numberOfComponents == 3 || numberOfComponents == 4)
		{
			texel.x = data[i + 0];
			texel.y = data[i + 1];
			texel.z = data[i + 2];

			if (numberOfComponents == 4)
			{
				texel.w = data[i + 3];
			}
		}

		m_texels_Float.push_back(texel);
	}
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

	if (m_imageType == IMAGE_TYPE_BYTE)
	{
		stbi_flip_vertically_on_write(1);
		stbi_write_png(fullFilename.c_str(), m_dimensions.x, m_dimensions.y, 4, GetTexelDataAsUChars(), 4 * m_dimensions.x);
	}
	else if (m_imageType == IMAGE_TYPE_FLOAT)
	{
		// As hdr
		stbi_flip_vertically_on_write(1);
		stbi_write_hdr(fullFilename.c_str(), m_dimensions.x, m_dimensions.y, 4, GetTexelDataAsFloats());
	}
	else
	{
		GUARANTEE_OR_DIE(false, "Invalid type to write to file");
	}
	 
}



void Image::SetImageType(eImageType imageType)
{
	m_imageType = imageType;
}



eImageType Image::GetImageType() const
{
	return m_imageType;
}

