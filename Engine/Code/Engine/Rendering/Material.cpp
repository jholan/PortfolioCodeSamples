#include "Engine/Rendering/Material.hpp"

#include <string>

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Sampler.hpp"
#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/ShaderUniformDescriptions.hpp"
#include "Engine/Rendering/PropertyBuffer.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Core/EngineCommon.h"
extern Renderer* g_theRenderer;



TextureSamplerType StringToTextureSamplerType(const std::string& samplerType, TextureSamplerType def)
{
	TextureSamplerType st = def;

	std::string s = ToLower(samplerType);
	if (s == "point")
	{
		st = TEXTURE_SAMPLER_TYPE_POINT;
	}
	else if (s == "linear")
	{
		st = TEXTURE_SAMPLER_TYPE_LINEAR;
	}
	else if (s == "point mipmap")
	{
		st = TEXTURE_SAMPLER_TYPE_POINT_MIPMAPS;
	}
	else if (s == "linear mipmap")
	{
		st = TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS;
	}

	return st;
}



// --------------------------------------------------------------------------------------------------------------------------------------
// Material Data-------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------------
static bool s_isInitialized = false;
void MaterialData::TryInitializeStatics()
{
	// First materials responsibility
	if (!s_isInitialized)
	{
		s_isInitialized = true;

		DEFUALT_DIFFUSE_TEXTURE = Texture::CreateOrGet("Data/BuiltIns/DEFAULT_TEXTURE.png", Texture::TEXTURE_FORMAT_RGBA8);
		DEFAULT_NORMAL_MAP = Texture::CreateOrGet("Data/BuiltIns/NORMAL_MAP_FLAT.png", Texture::TEXTURE_FORMAT_RGBA8);

		Sampler* pointSampler = new Sampler();
		pointSampler->Initialize(false);
		POINT_SAMPLER = pointSampler;

		Sampler* linearSampler = new Sampler();
		linearSampler->Initialize(true);
		LINEAR_SAMPLER = linearSampler;

		Sampler* pointMipmapSampler = new Sampler();
		pointMipmapSampler->Initialize(false, true);
		POINT_MIPMAP_SAMPLER = pointMipmapSampler;

		Sampler* linearMipmapSampler = new Sampler();
		linearMipmapSampler->Initialize(true, true);
		LINEAR_MIPMAP_SAMPLER = linearMipmapSampler;
	}
}



PropertyBlock* MaterialData::CreateOrGetPropertyBuffer(const PropertyBlockDescription* description)
{
	PropertyBlock* propertyBuffer = nullptr;

	// Try to find the desired PropertBuffer
	for (int i = 0; i < (int)m_propertyBuffers.size(); ++i)
	{
		if (m_propertyBuffers[i]->GetName() == description->GetName())
		{
			propertyBuffer = m_propertyBuffers[i];
			break;
		}
	}

	// If we couldn't find it
	if (propertyBuffer == nullptr)
	{
		// Create a new one
		propertyBuffer = new PropertyBlock();
		propertyBuffer->SetSize(description->GetSizeInBytes());
		propertyBuffer->SetDescription(description);
		m_propertyBuffers.push_back(propertyBuffer);
	}

	return propertyBuffer;
}



MaterialData::MaterialData()
{
	TryInitializeStatics();

	SetDiffuseTexture(nullptr);
	SetNormalMap(nullptr);
}



MaterialData::MaterialData(const MaterialData& toCopy)
{
	TryInitializeStatics();

	SetShader(toCopy.m_shader);
	m_textures = toCopy.m_textures;
	m_samplers = toCopy.m_samplers;

	m_propertyBuffers.reserve(toCopy.m_propertyBuffers.size());
	for (int i = 0; i < (int)toCopy.m_propertyBuffers.size(); ++i)
	{
		PropertyBlock* propertyBufferCopy = new PropertyBlock(*(toCopy.m_propertyBuffers[i]));
		m_propertyBuffers.push_back(propertyBufferCopy);
	}
}



MaterialData::MaterialData(const XMLElement& definition)
{
	// Base
	TryInitializeStatics();

	SetDiffuseTexture(nullptr);
	SetNormalMap(nullptr);



	// Extend materials
	if (DoesXMLAttributeExist(definition, "base"))
	{
		std::string baseMaterialName = ParseXmlAttribute(definition, "base", "");
		MaterialData* baseMaterial = Get(baseMaterialName);
		TODO("Better material base class support? How often do we use this?");
		GUARANTEE_OR_DIE(baseMaterial != nullptr, "Base material not found, Check spelling or make sure the base class is above this in the file");

		*this = *baseMaterial;
	}

	// Shader
	const XMLElement* shaderElement = definition.FirstChildElement("Shader");
	if (shaderElement)
	{
		if (DoesXMLAttributeExist(*shaderElement, "name"))
		{
			SetShader(ParseXmlAttribute(*shaderElement, "name", ""));
		}
	}


	// Textures
	const XMLElement* textureElement = definition.FirstChildElement("Texture");
	while(textureElement != nullptr)
	{
		// Required Attributes
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*textureElement, "bindPoint"), "Material textures mush have a bindPoint specified");
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*textureElement, "filepath"),  "Material textures mush have a filepath specified");


		// Bind Point
		int bindPoint = ParseXmlAttribute(*textureElement, "bindPoint", 0);
		

		// Filepath
		std::string filepath = ParseXmlAttribute(*textureElement, "filepath", "");
		

		// Sampler(Optional)
		TextureSamplerType samplerType = StringToTextureSamplerType(ParseXmlAttribute(*textureElement, "sampler", ""), TEXTURE_SAMPLER_TYPE_LINEAR);


		// Texture Format
		unsigned int textureFormat = Texture::StringToTextureFormat(ParseXmlAttribute(*textureElement, "format", ""), Texture::TEXTURE_FORMAT_RGBA8);


		// Add it
		SetTexture(bindPoint, filepath, textureFormat, samplerType);


		// Get next Texture element
		textureElement = textureElement->NextSiblingElement("Texture");
	}

	// Helper textures
	const XMLElement* diffuseElement = definition.FirstChildElement("DiffuseTexture");
	if (diffuseElement != nullptr)
	{
		// Required Attributes
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*diffuseElement, "filepath"),  "Material textures mush have a filepath specified");


		// Filepath
		std::string filepath = ParseXmlAttribute(*diffuseElement, "filepath", "");
		
		// Sampler
		TextureSamplerType samplerType = StringToTextureSamplerType(ParseXmlAttribute(*diffuseElement, "sampler", ""), TEXTURE_SAMPLER_TYPE_LINEAR);
		

		// Texture Format
		unsigned int textureFormat = Texture::StringToTextureFormat(ParseXmlAttribute(*diffuseElement, "format", ""), Texture::TEXTURE_FORMAT_RGBA8);

		
		// Add it
		SetDiffuseTexture(filepath, textureFormat, samplerType);
	}

	const XMLElement* normalMapElement = definition.FirstChildElement("NormalMap");
	if (normalMapElement != nullptr)
	{
		// Required Attributes
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*normalMapElement, "filepath"),  "Material textures mush have a filepath specified");


		// Filepath
		std::string filepath = ParseXmlAttribute(*normalMapElement, "filepath", "");
		
		// Sampler
		TextureSamplerType samplerType = StringToTextureSamplerType(ParseXmlAttribute(*normalMapElement, "sampler", ""), TEXTURE_SAMPLER_TYPE_LINEAR);


		// Texture Format
		unsigned int textureFormat = Texture::StringToTextureFormat(ParseXmlAttribute(*normalMapElement, "format", ""), Texture::TEXTURE_FORMAT_RGBA8);
		
		
		// Add it
		SetNormalMap(filepath, textureFormat, samplerType);
	}


	// Property
	const XMLElement* propertyElement = definition.FirstChildElement("Property");
	while(propertyElement != nullptr)
	{
		// Make sure required elements are present
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*propertyElement, "name"), "Material properties mush have a name specified");
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*propertyElement, "type"), "Material properties mush have a type specified");
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*propertyElement, "value"), "Material properties mush have a value specified");

		// Load property based on type
		std::string propName = ParseXmlAttribute(*propertyElement, "name", "");
		std::string propValue = ParseXmlAttribute(*propertyElement, "value", "");
		std::string propType = ParseXmlAttribute(*propertyElement, "type", "");
		
		propType = ToLower(propType);
		if(propType == "int")
		{
			SetProperty(propName, StringToInt(propValue.c_str()));
		}
		else if(propType == "unsigned int" || propType == "uint")
		{
			SetProperty(propName, (unsigned int)StringToInt(propValue.c_str()));
		}
		else if(propType == "float")
		{
			SetProperty(propName, StringToFloat(propValue.c_str()));
		}
		else if(propType == "vector2")
		{
			Vector2 val = Vector2();
			val.SetFromText(propValue.c_str());
			SetProperty(propName, val);
		}
		else if(propType == "vector3")
		{
			Vector3 val = Vector3();
			val.SetFromText(propValue.c_str());
			SetProperty(propName, val);
		}
		else if(propType == "vector4")
		{
			Vector4 val = Vector4();
			val.SetFromText(propValue.c_str());
			SetProperty(propName, val);
		}
		else if(propType == "matrix4")
		{
			Matrix4 val = Matrix4();
			val.SetFromText(propValue.c_str());
			SetProperty(propName, val);
		}
		else if(propType == "rgba")
		{
			RGBA val = RGBA();
			val.SetFromText(propValue.c_str());
			SetProperty(propName, val);
		}


		propertyElement = propertyElement->NextSiblingElement("Property");
	}
}



MaterialData::~MaterialData()
{
	for (int i = 0; i < (int)m_propertyBuffers.size(); ++i)
	{
		delete m_propertyBuffers[i];
		m_propertyBuffers[i] = nullptr;
	}
	m_propertyBuffers.clear();
}



MaterialData* MaterialData::Clone() const
{
	MaterialData* clonedData = new MaterialData(*this);
	return clonedData;
}



MaterialData& MaterialData::operator=( const MaterialData& toCopy)
{
	TryInitializeStatics();

	SetShader(toCopy.m_shader);
	m_textures = toCopy.m_textures;
	m_samplers = toCopy.m_samplers;

	m_propertyBuffers.reserve(toCopy.m_propertyBuffers.size());
	for (int i = 0; i < (int)toCopy.m_propertyBuffers.size(); ++i)
	{
		PropertyBlock* propertyBufferCopy = new PropertyBlock(*(toCopy.m_propertyBuffers[i]));
		m_propertyBuffers.push_back(propertyBufferCopy);
	}

	return *this;
}



void MaterialData::SetShader(const std::string& name)
{
	const Shader* shader = Shader::Get(name);
	SetShader(shader);
}



void MaterialData::SetShader(const Shader* shader)
{
	m_shader = shader;
}



const Shader* MaterialData::GetShader() const
{
	return m_shader;
}



void MaterialData::SetTexture(unsigned int bindPoint, const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType)
{
	// If we are using a mip map sampler we want to generate mip maps for our texture
	bool generateMipmaps = false;
	if (samplerType == TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS || samplerType == TEXTURE_SAMPLER_TYPE_POINT_MIPMAPS)
	{
		generateMipmaps = true;
	}

	SetTexture( bindPoint, Texture::CreateOrGet(filepath, textureFormat, generateMipmaps), samplerType);
}


// The Internal function that does the "work"
void MaterialData::SetTexture(unsigned int bindPoint, const Texture* texture, TextureSamplerType samplerType)
{
	EnsureTextureArrayCanHoldXTextures(bindPoint);
	m_textures[bindPoint] = texture;
	SetSampler(bindPoint, samplerType);
}


unsigned int MaterialData::GetNumberOfTextures() const
{
	return m_textures.size();
}



bool MaterialData::IsValidTexture(unsigned int bindPoint) const
{
	bool isValid = false;

	if (m_textures[bindPoint] != nullptr)
	{
		isValid = true;
	}

	return isValid;
}



const Texture* MaterialData::GetTexture(unsigned int bindPoint) const
{
	return m_textures[bindPoint];
}



const Sampler* MaterialData::GetSampler(unsigned int bindPoint) const
{
	return m_samplers[bindPoint];
}



void MaterialData::SetDiffuseTexture(const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType)
{
	// If we are using a mip map sampler we want to generate mip maps for our texture
	bool generateMipmaps = false;
	if (samplerType == TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS || samplerType == TEXTURE_SAMPLER_TYPE_POINT_MIPMAPS)
	{
		generateMipmaps = true;
	}

	if (!filepath.empty())
	{
		SetDiffuseTexture(Texture::CreateOrGet(filepath, textureFormat, generateMipmaps), samplerType);
	}
	else
	{
		SetDiffuseTexture(nullptr, samplerType);
	}
}



void MaterialData::SetDiffuseTexture(const Texture* texture, TextureSamplerType samplerType)
{
	if (texture == nullptr)
	{
		texture = DEFUALT_DIFFUSE_TEXTURE;
	}
	SetTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, texture, samplerType);
}



const Texture* MaterialData::GetDiffuseTexture() const
{
	return m_textures[Renderer::DIFFUSE_TEXTURE_BIND_POINT];
}



void MaterialData::SetNormalMap(const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType)
{
	// If we are using a mip map sampler we want to generate mip maps for our texture
	bool generateMipmaps = false;
	if (samplerType == TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS || samplerType == TEXTURE_SAMPLER_TYPE_POINT_MIPMAPS)
	{
		generateMipmaps = true;
	}

	if (!filepath.empty())
	{
		SetNormalMap(Texture::CreateOrGet(filepath, textureFormat, generateMipmaps), samplerType);
	}
	else
	{
		SetNormalMap(nullptr, samplerType);
	}
}



void MaterialData::SetNormalMap(const Texture* texture, TextureSamplerType samplerType)
{
	if (texture == nullptr)
	{
		texture = DEFAULT_NORMAL_MAP;
	}
	SetTexture(Renderer::NORMAL_MAP_BIND_POINT, texture, samplerType);
}



const Texture* MaterialData::GetNormalMap() const
{
	return m_textures[Renderer::NORMAL_MAP_BIND_POINT];

}



void MaterialData::SetSampler(unsigned int bindPoint, TextureSamplerType samplerType)
{
	const Sampler* sampler = nullptr;

 	switch(samplerType)
	{
	case TEXTURE_SAMPLER_TYPE_POINT:
	{
		sampler = POINT_SAMPLER;
		break;
	}
	case TEXTURE_SAMPLER_TYPE_LINEAR:
	{
		sampler = LINEAR_SAMPLER;
		break;
	}
	case TEXTURE_SAMPLER_TYPE_POINT_MIPMAPS:
	{
		sampler = POINT_MIPMAP_SAMPLER;
		break;
	}
	case TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS:
	{
		sampler = LINEAR_MIPMAP_SAMPLER;
		break;
	}
	default:
	{
		sampler = LINEAR_SAMPLER;
		break;
	}
	}

	m_samplers[bindPoint] = sampler;
}



void MaterialData::EnsureTextureArrayCanHoldXTextures(unsigned int count)
{
	unsigned int targetSize = count + 1;
	if (m_textures.size() >= targetSize)
	{
		// We are correctly sized
	}
	else
	{
		// resize and fill with nullptr
		unsigned int currSize = (unsigned int)m_textures.size();
		m_textures.reserve(targetSize);
		m_samplers.reserve(targetSize);
		for (unsigned int i = currSize; i < targetSize; ++i)
		{
			m_textures.push_back(nullptr);
			m_samplers.push_back(nullptr);
		}
	}
}



void MaterialData::SetProperty(const std::string& property, int	value)
{
	SetProperty(property, (void*)(&value), sizeof(int));
}



void MaterialData::SetProperty(const std::string& property, unsigned int value)
{
	SetProperty(property, (void*)(&value), sizeof(unsigned int));
}



void MaterialData::SetProperty(const std::string& property, float value)
{
	SetProperty(property, (void*)(&value), sizeof(float));
}



void MaterialData::SetProperty(const std::string& property, const Vector2& value)
{
	SetProperty(property, (void*)(&value), sizeof(Vector2));
}



void MaterialData::SetProperty(const std::string& property, const Vector3& value)
{
	SetProperty(property, (void*)(&value), sizeof(Vector3));
}



void MaterialData::SetProperty(const std::string& property, const Vector4& value)
{
	SetProperty(property, (void*)(&value), sizeof(Vector4));
}



void MaterialData::SetProperty(const std::string& property, const Matrix4& value)
{
	SetProperty(property, (void*)(&value), sizeof(Matrix4));
}



void MaterialData::SetProperty(const std::string& property, const RGBA& value)
{
	Vector4 color = value.GetAsVector4();
	SetProperty(property, (void*)(&color), sizeof(Vector4));
}



void MaterialData::SetProperty(const std::string& property, const void* data, unsigned int dataSize)
{
	const ShaderMaterialPropertyDescription* materialDescription = GetShader()->GetShaderProgram()->GetShaderMaterialDescription();

	const PropertyDescription* propertyDescription = materialDescription->GetPropertyDescription(property);
	GUARANTEE_OR_DIE(propertyDescription != nullptr, Stringf("Material Property %s does not exist", property.c_str()).c_str());

	const PropertyBlockDescription* propertyBufferDescription = propertyDescription->GetContainingBuffer();

	PropertyBlock* propertyBuffer = CreateOrGetPropertyBuffer(propertyBufferDescription);

	GUARANTEE_OR_DIE(propertyDescription->GetSizeInBytes() == dataSize, Stringf("Property %s size was different than the size in the shader", property.c_str()).c_str());
	propertyBuffer->RawSet(propertyDescription->GetOffsetIntoContainingBufferInBytes(), data, dataSize);
}


void MaterialData::BindPropertyBuffers() const
{
	for (int i = 0; i < (int)m_propertyBuffers.size(); ++i)
	{
		m_propertyBuffers[i]->Bind();
	}
}



MaterialData* MaterialData::Get(const std::string& name)
{
	MaterialData* mat = nullptr;

	auto posInMap = s_loadedMaterialDatas.find(name);
	if (posInMap == s_loadedMaterialDatas.end())
	{
		// We have a bad material
		// Come up with a good bad material
			// Unlit MVP?
			// Difftexture = BAD_MATERIAL, point
		mat = DEFAULT_MATERIAL;
	}
	else
	{
		mat = s_loadedMaterialDatas[name];
	}

	return mat;
}



MaterialData* MaterialData::FromShader(const std::string& shaderName)
{
	MaterialData* mat = new MaterialData();
	mat->SetShader(shaderName);

	return mat;
}



// Issues do we cant to register a copy or the pointer?
//	if pointer we need to manage the memory now
// for update do we want to copy the interior or again point to the new material?
//	if point to new must free old and manage new
void MaterialData::RegisterMaterialData(const std::string& name, MaterialData* material, bool update)
{
	auto posInMap = s_loadedMaterialDatas.find(name);
	if (posInMap == s_loadedMaterialDatas.end())
	{
		// We do not know about this material, lets add it
		s_loadedMaterialDatas[name] = material->Clone();
	}
	else
	{
		if (update == true)
		{
			// We already have a texture with this name, but we want to update it anyway


		}
	}
}



void MaterialData::ParseMaterialDatas(const std::string& materialFile)
{
	if (DEFAULT_MATERIAL == nullptr)
	{
		DEFAULT_MATERIAL = MaterialData::FromShader("DebugDraw_3D_depth");
		DEFAULT_MATERIAL->SetDiffuseTexture("Data/BuiltIns/BAD_MATERIAL.png", Texture::TEXTURE_FORMAT_RGBA8);
	}

	DEFUALT_DIFFUSE_TEXTURE;

	XMLDocument doc;
	doc.LoadFile(materialFile.c_str());
	XMLElement* shadersElement = doc.FirstChildElement("Materials");


	const XMLElement* matDataElement = shadersElement->FirstChildElement("Material");
	while(matDataElement != nullptr)
	{
		std::string name = ParseXmlAttribute(*matDataElement, "name", "");
		if (!name.empty())
		{
			// If we have a name.
			auto locationInDefinitions = s_loadedMaterialDatas.find(name);
			if (locationInDefinitions == s_loadedMaterialDatas.end())
			{
				// And we are not already defined.
				MaterialData* matData = new MaterialData(*matDataElement);
				s_loadedMaterialDatas[name] = matData;
			}
		}

		matDataElement = matDataElement->NextSiblingElement("Material");
	}



	doc.Clear();
}

std::map<std::string, MaterialData*> MaterialData::s_loadedMaterialDatas;

const Texture* MaterialData::DEFUALT_DIFFUSE_TEXTURE = nullptr;
const Texture* MaterialData::DEFAULT_NORMAL_MAP		 = nullptr;

const Sampler* MaterialData::POINT_SAMPLER			 = nullptr;
const Sampler* MaterialData::LINEAR_SAMPLER			 = nullptr;
const Sampler* MaterialData::POINT_MIPMAP_SAMPLER	 = nullptr;
const Sampler* MaterialData::LINEAR_MIPMAP_SAMPLER	 = nullptr;

MaterialData* MaterialData::DEFAULT_MATERIAL		 = nullptr;




// --------------------------------------------------------------------------------------------------------------------------------------
// Material------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------------
Material::Material()
{
}



Material::~Material()
{
	if (m_mutableMaterial != nullptr)
	{
		delete m_mutableMaterial;
		m_mutableMaterial = nullptr;
	}
}



// Shader
void Material::SetShader(const std::string& name)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetShader(name);
}



void Material::SetShader(const Shader* shader)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetShader(shader);
}



const Shader* Material::GetShader() const
{
	return GetActiveMaterial()->GetShader();
}



void Material::SetTexture(unsigned int bindPoint, const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetTexture(bindPoint, filepath, textureFormat, samplerType);
}



void Material::SetTexture(unsigned int bindPoint, const Texture* texture, TextureSamplerType samplerType)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetTexture(bindPoint, texture, samplerType);
}



void Material::SetDiffuseTexture(const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetDiffuseTexture(filepath, textureFormat, samplerType);
}



void Material::SetDiffuseTexture(const Texture* texture, TextureSamplerType samplerType)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetDiffuseTexture(texture, samplerType);
}



void Material::SetNormalMap(const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetNormalMap(filepath, textureFormat, samplerType);
}



void Material::SetNormalMap(const Texture* texture, TextureSamplerType samplerType)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetNormalMap(texture, samplerType);
}



const Sampler* Material::GetSampler(unsigned int bindPoint) const
{
	return GetActiveMaterial()->GetSampler(bindPoint);
}



bool Material::IsValidTexture(unsigned int bindPoint) const
{
	return GetActiveMaterial()->IsValidTexture(bindPoint);
}



const Texture* Material::GetTexture(unsigned int bindPoint) const
{
	return GetActiveMaterial()->GetTexture(bindPoint);
}



const Texture* Material::GetDiffuseTexture() const
{
	return GetActiveMaterial()->GetDiffuseTexture();
}



const Texture* Material::GetNormalMap() const
{
	return GetActiveMaterial()->GetNormalMap();
}



unsigned int Material::GetNumberOfTextures() const
{
	return GetActiveMaterial()->GetNumberOfTextures();
}



void Material::SetProperty(const std::string& property, int value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::SetProperty(const std::string& property, unsigned int value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::SetProperty(const std::string& property, float value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::SetProperty(const std::string& property, const Vector2& value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::SetProperty(const std::string& property, const Vector3& value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::SetProperty(const std::string& property, const Vector4& value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::SetProperty(const std::string& property, const Matrix4& value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::SetProperty(const std::string& property, const RGBA& value)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetProperty(property, value);
}



void Material::BindPropertyBuffers() const
{
	GetActiveMaterial()->BindPropertyBuffers();
}



Material* Material::Get(const std::string& name)
{
	Material* mat = new Material();
	mat->m_sharedMaterial = MaterialData::Get(name);

	return mat;
}



Material* Material::FromShader(const std::string& shaderName)
{
	Material* mat = new Material();

	mat->SetShader(shaderName); // Automatically makes the material mutable

	return mat;
}



void Material::RegisterMaterial(const std::string& name, Material* material, bool update)
{
	UNIMPLEMENTED();
	UNUSED(name);
	UNUSED(material);
	UNUSED(update);
}



void Material::ParseMaterials(const std::string& materialFile)
{
	MaterialData::ParseMaterialDatas(materialFile);
}



MaterialData* Material::GetActiveMaterial()
{
	MaterialData* matData = nullptr;

	if (m_mutableMaterial != nullptr)
	{
		matData = m_mutableMaterial;
	}
	else
	{
		matData = m_sharedMaterial;
	}

	return matData;
}



const MaterialData* Material::GetActiveMaterial() const
{
	const MaterialData* matData = nullptr;

	if (m_mutableMaterial != nullptr)
	{
		matData = m_mutableMaterial;
	}
	else
	{
		matData = m_sharedMaterial;
	}

	return matData;
}



void Material::TryAllocateMutableMaterial()
{
	// Short circuit if we already have a mutable material allocated.
	if (m_mutableMaterial != nullptr)
	{
		return;
	}

	// Allocate one
	if (m_sharedMaterial == nullptr)
	{
		// For the case where we make a material mutable from the get go A la FromShader
		// We have no base shader
		m_mutableMaterial = new MaterialData();
	}
	else
	{
		m_mutableMaterial = m_sharedMaterial->Clone();
	}
}
