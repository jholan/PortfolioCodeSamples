#include "Engine/Rendering/Material.hpp"

#include <string>

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Rendering/Texture2D.hpp"
#include "Engine/Rendering/TextureView.hpp"
#include "Engine/Rendering/Sampler.hpp"
#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/ShaderUniformDescriptions.hpp"
#include "Engine/Rendering/PropertyBuffer.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Core/EngineCommon.h"
extern Renderer* g_theRenderer;



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
	}
}



PropertyBlock* MaterialData::CreateOrGetPropertyBuffer(const PropertyBufferDescription* description)
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
}



MaterialData::MaterialData(const MaterialData& toCopy)
{
	TryInitializeStatics();

	SetShader(toCopy.m_shader);


	m_textureViews.reserve(toCopy.m_textureViews.size());
	for (int i = 0; i < (int)toCopy.m_textureViews.size(); ++i)
	{
		SetTexture(i, toCopy.m_textureViews[i]->GetTextureBuffer<Texture2D>());
	}


	m_samplers.reserve(toCopy.m_samplers.size());
	for (int i = 0; i < (int)toCopy.m_textureViews.size(); ++i)
	{
		SetSampler(i, toCopy.m_samplers[i]);
	}


	m_propertyBuffers.reserve(toCopy.m_propertyBuffers.size());
	for (int i = 0; i < (int)toCopy.m_propertyBuffers.size(); ++i)
	{
		PropertyBlock* propertyBufferCopy = new PropertyBlock(*(toCopy.m_propertyBuffers[i]));
		m_propertyBuffers.push_back(propertyBufferCopy);
	}
}



MaterialData::MaterialData(const XMLEle& definition)
{
	// Base
	TryInitializeStatics();


	// Extend materials
	if (DoesXMLAttributeExist(definition, "base"))
	{
		std::string baseMaterialName = ParseXmlAttribute(definition, "base", "");
		MaterialData* baseMaterial = Get(baseMaterialName);
		TODO("Better material base class support? How often do we use this?");
		GUARANTEE_OR_DIE(baseMaterial != nullptr, "Base material not found, Check spelling or make sure the base class is above this in the file");

		*this = *baseMaterial;
	}


	// Shader --------------------------------------------------------------------------------
	const XMLEle* shaderElement = definition.FirstChildElement("Shader");
	if (shaderElement)
	{
		if (DoesXMLAttributeExist(*shaderElement, "name"))
		{
			SetShader(ParseXmlAttribute(*shaderElement, "name", ""));
		}
	}


	// Textures -------------------------------------------------------------------------------
	const XMLEle* textureElement = definition.FirstChildElement("Texture");
	while(textureElement != nullptr)
	{
		// Required Attributes
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*textureElement, "bindPoint"),	"Material textures must have a bindPoint specified");
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*textureElement, "name"),		"Material textures must have a name specified");


		// Bind Point
		int bindPoint = ParseXmlAttribute(*textureElement, "bindPoint", 0);
		
		// Filepath
		std::string textureName = ParseXmlAttribute(*textureElement, "name", "");
		
		// Add it
		SetTexture(bindPoint, textureName);


		// Get next Texture element
		textureElement = textureElement->NextSiblingElement("Texture");
	}


	// Sampler -------------------------------------------------------------------------------
	const XMLEle* samplerElement = definition.FirstChildElement("Sampler");
	while(samplerElement != nullptr)
	{
		// Required Attributes
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*samplerElement, "bindPoint"),	"Material samplers must have a bindPoint specified");
		GUARANTEE_OR_DIE(DoesXMLAttributeExist(*samplerElement, "name"),		"Material samplers must have a name specified");


		// Bind Point
		int bindPoint = ParseXmlAttribute(*samplerElement, "bindPoint", 0);

		// Filepath
		std::string samplerName = ParseXmlAttribute(*samplerElement, "name", "");
		const Sampler* sampler = StringToSampler(samplerName);

		// Add it
		SetSampler(bindPoint, sampler);


		// Get next Sampler element
		samplerElement = samplerElement->NextSiblingElement("Sampler");
	}


	// Property
	const XMLEle* propertyElement = definition.FirstChildElement("Property");
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
	for (int i = 0; i < (int)m_textureViews.size(); ++i)
	{
		m_textureViews[i]->Destroy();
		delete m_textureViews[i];
		m_textureViews[i] = nullptr;
	}
	m_textureViews.clear();


	m_samplers.clear();


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
	m_textureViews = toCopy.m_textureViews;
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



void MaterialData::SetTexture(unsigned int bindPoint, const std::string& name)
{
	SetTexture(bindPoint, Texture2D::Get(name));
}



void MaterialData::AddSRVToBindPoint(unsigned int bindPoint, const Texture2D* texture)
{
	EnsureTextureArrayCanHoldXTextures(bindPoint);

	RemoveSRVFromBindPoint(bindPoint);

	m_textureViews[bindPoint] = new ShaderResourceView();
	m_textureViews[bindPoint]->Initialize(texture);
}



void MaterialData::RemoveSRVFromBindPoint(unsigned int bindPoint)
{
	if (m_textureViews[bindPoint] != nullptr)
	{
		m_textureViews[bindPoint]->Destroy();
		delete m_textureViews[bindPoint];
		m_textureViews[bindPoint] = nullptr;
	}
}


// The Internal function that does the "work"
void MaterialData::SetTexture(unsigned int bindPoint, const Texture2D* texture)
{
	AddSRVToBindPoint(bindPoint, texture);
}


unsigned int MaterialData::GetNumberOfTextures() const
{
	return (uint)m_textureViews.size();
}



bool MaterialData::IsValidTexture(unsigned int bindPoint) const
{
	bool isValid = false;

	if (m_textureViews[bindPoint] != nullptr)
	{
		isValid = true;
	}

	return isValid;
}



const ShaderResourceView* MaterialData::GetTexture(unsigned int bindPoint) const
{
	return m_textureViews[bindPoint];
}



const Sampler* MaterialData::GetSampler(unsigned int bindPoint) const
{
	return m_samplers[bindPoint];
}



bool MaterialData::IsValidSampler( unsigned int bindPoint) const
{
	bool isValid = false;

	if (m_samplers[bindPoint] != nullptr)
	{
		isValid = true;
	}

	return isValid;
}



unsigned int MaterialData::GetNumberOfSamplers() const
{
	return (uint)m_samplers.size();
}



void MaterialData::SetSampler(unsigned int bindPoint, const Sampler* sampler)
{
	EnsureSamplerArrayCanHoldXSamplers(bindPoint);

	m_samplers[bindPoint] = sampler;
}



void MaterialData::EnsureTextureArrayCanHoldXTextures(unsigned int count)
{
	unsigned int targetSize = count + 1;
	if (m_textureViews.size() >= targetSize)
	{
		// We are correctly sized
	}
	else
	{
		// resize and fill with nullptr
		unsigned int currSize = (unsigned int)m_textureViews.size();
		m_textureViews.reserve(targetSize);
		for (unsigned int i = currSize; i < targetSize; ++i)
		{
			m_textureViews.push_back(nullptr);
		}
	}
}



void MaterialData::EnsureSamplerArrayCanHoldXSamplers(unsigned int count)
{
	unsigned int targetSize = count + 1;
	if (m_samplers.size() >= targetSize)
	{
		// We are correctly sized
	}
	else
	{
		// resize and fill with nullptr
		unsigned int currSize = (unsigned int)m_samplers.size();
		m_samplers.reserve(targetSize);
		for (unsigned int i = currSize; i < targetSize; ++i)
		{
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
	
	if (propertyDescription == nullptr)
	{
		DebugDraw_Log(30.0f, RGBA(1,0,0), "Material Property - %s does not exist", property.c_str());
		return;
	}

	const PropertyBufferDescription* propertyBufferDescription = propertyDescription->GetContainingBuffer();

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
		GUARANTEE_OR_DIE(false, Stringf("No Material with name = %s found", name.c_str()).c_str());
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
		DEFAULT_MATERIAL->SetTexture(0, "BAD_MATERIAL");
	}

	XMLDoc doc;
	doc.LoadFile(materialFile.c_str());
	XMLEle* shadersElement = doc.FirstChildElement("Materials");


	const XMLEle* matDataElement = shadersElement->FirstChildElement("Material");
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

MaterialData* MaterialData::DEFAULT_MATERIAL			= nullptr;




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



void Material::SetTexture(unsigned int bindPoint, const std::string& name)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetTexture(bindPoint, name);
}



void Material::SetTexture(unsigned int bindPoint, const Texture2D* texture)
{
	TryAllocateMutableMaterial();
	GetActiveMaterial()->SetTexture(bindPoint, texture);
}



void Material::SetSampler(unsigned int bindPoint, const Sampler* sampler)
{
	return GetActiveMaterial()->SetSampler(bindPoint, sampler);
}



const Sampler* Material::GetSampler(unsigned int bindPoint) const
{
	return GetActiveMaterial()->GetSampler(bindPoint);
}



bool Material::IsValidSampler(unsigned int bindPoint) const
{
	return GetActiveMaterial()->IsValidSampler(bindPoint);
}



unsigned int Material::GetNumberOfSamplers() const
{
	return GetActiveMaterial()->GetNumberOfSamplers();
}



bool Material::IsValidTexture(unsigned int bindPoint) const
{
	return GetActiveMaterial()->IsValidTexture(bindPoint);
}



const ShaderResourceView* Material::GetTexture(unsigned int bindPoint) const
{
	return GetActiveMaterial()->GetTexture(bindPoint);
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
