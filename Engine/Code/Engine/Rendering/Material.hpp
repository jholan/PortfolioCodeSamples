#pragma once

#include <vector>
#include <string>
#include <map>

#include "Engine/Core/XmlUtilities.hpp"

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix4.hpp"

class Shader;
class Texture;
class Sampler;
class MaterialProperty;
class PropertyBlock;
class PropertyBlockDescription;



enum TextureSamplerType
{
	TEXTURE_SAMPLER_TYPE_INVALID = -1,
				  
	TEXTURE_SAMPLER_TYPE_POINT = 0,
	TEXTURE_SAMPLER_TYPE_LINEAR,

	TEXTURE_SAMPLER_TYPE_POINT_MIPMAPS,
	TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS,
				  
	TEXTURE_SAMPLER_TYPE_COUNT
};
TextureSamplerType StringToTextureSamplerType(const std::string& samplerType, TextureSamplerType def = TEXTURE_SAMPLER_TYPE_LINEAR);



class MaterialData
{
public:
	MaterialData();
	MaterialData(const MaterialData& toCopy);
	MaterialData(const XMLElement& definition);
	~MaterialData();
	MaterialData* Clone() const;
	MaterialData& operator=( const MaterialData& toCopy);

	// Shader
	void				 SetShader(const std::string& name);
	void				 SetShader(const Shader* shader);
	const Shader*		 GetShader() const;

	// Textures
	void				 SetTexture(unsigned int bindPoint, const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
	void				 SetTexture(unsigned int bindPoint, const Texture*	   texture,	 TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
						 
	void				 SetDiffuseTexture(const std::string& filepath, unsigned int textureFormat, TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
	void				 SetDiffuseTexture(const Texture*	  texture,  TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
						 
	void				 SetNormalMap(const std::string&	  filepath, unsigned int textureFormat, TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
	void				 SetNormalMap(const Texture*		  texture,  TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);

	// Texture Access
	const Sampler*		 GetSampler(unsigned int bindPoint) const;
	bool				 IsValidTexture( unsigned int bindPoint) const;
	const Texture*		 GetTexture(unsigned int bindPoint) const;
	const Texture*		 GetDiffuseTexture() const;
	const Texture*		 GetNormalMap() const;
	unsigned int		 GetNumberOfTextures() const;

	// Properties
	void				 SetProperty(const std::string& property, int			 value);
	void				 SetProperty(const std::string& property, unsigned int   value);
	void				 SetProperty(const std::string& property, float			 value);
	void				 SetProperty(const std::string& property, const Vector2& value);
	void				 SetProperty(const std::string& property, const Vector3& value);
	void				 SetProperty(const std::string& property, const Vector4& value);
	void				 SetProperty(const std::string& property, const Matrix4& value);
	void				 SetProperty(const std::string& property, const RGBA&    value);
	void				 SetProperty(const std::string& property, const void*	 data,  unsigned int dataSize);
	void				 BindPropertyBuffers() const;



	static MaterialData* Get(const std::string& name);
	static MaterialData* FromShader(const std::string& shaderName);
	static void			 RegisterMaterialData(const std::string& name, MaterialData* material, bool update = false);
	static void			 ParseMaterialDatas(const std::string& materialFile);


private:
	void				 TryInitializeStatics();
	PropertyBlock*		 CreateOrGetPropertyBuffer(const PropertyBlockDescription* description);

	// Textures
	void				 SetSampler(unsigned int bindPoint, TextureSamplerType samplerType);
	void				 EnsureTextureArrayCanHoldXTextures(unsigned int count);

	const Shader*					m_shader;
	std::vector<const Texture*>		m_textures;
	std::vector<const Sampler*>		m_samplers;
	std::vector<PropertyBlock*>		m_propertyBuffers;



	static std::map<std::string, MaterialData*> s_loadedMaterialDatas;

	static const Texture*						DEFUALT_DIFFUSE_TEXTURE;
	static const Texture*						DEFAULT_NORMAL_MAP;

	static const Sampler*						POINT_SAMPLER;
	static const Sampler*						LINEAR_SAMPLER;
	static const Sampler*						POINT_MIPMAP_SAMPLER;
	static const Sampler*						LINEAR_MIPMAP_SAMPLER;
	// TODO Mip Samplers

	static MaterialData*						DEFAULT_MATERIAL;
};



class Material
{
public:
	Material();
	~Material();

	// Shader
	void			SetShader(const std::string& name);
	void			SetShader(const Shader* shader);
	const Shader*	GetShader() const;

	// Textures
	void			SetTexture(unsigned int bindPoint, const std::string& filepath,  unsigned int textureFormat, TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
	void			SetTexture(unsigned int bindPoint, const Texture*	   texture,	 TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);

	void			SetDiffuseTexture(const std::string&  filepath,  unsigned int textureFormat, TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
	void			SetDiffuseTexture(const Texture*	  texture,   TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);

	void			SetNormalMap(const std::string&	  filepath,		unsigned int textureFormat, TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);
	void			SetNormalMap(const Texture*		  texture,		TextureSamplerType samplerType = TEXTURE_SAMPLER_TYPE_LINEAR);

	// Texture Access
	const Sampler*	GetSampler(		unsigned int bindPoint) const;
	bool			IsValidTexture( unsigned int bindPoint) const;
	const Texture*	GetTexture(		unsigned int bindPoint) const;
	const Texture*	GetDiffuseTexture() const;
	const Texture*	GetNormalMap() const;
	unsigned int	GetNumberOfTextures() const;

	// Properties
	void			SetProperty(const std::string& property, int			 value);
	void			SetProperty(const std::string& property, unsigned int   value);
	void			SetProperty(const std::string& property, float			 value);
	void			SetProperty(const std::string& property, const Vector2& value);
	void			SetProperty(const std::string& property, const Vector3& value);
	void			SetProperty(const std::string& property, const Vector4& value);
	void			SetProperty(const std::string& property, const Matrix4& value);
	void			SetProperty(const std::string& property, const RGBA&    value);
	void			BindPropertyBuffers() const;



	static Material* Get(const std::string& name);
	static Material* FromShader(const std::string& shaderName); // Automatically mutable
	static void		 RegisterMaterial(const std::string& name, Material* material, bool update = false);
	static void		 ParseMaterials(const std::string& materialFile);


	MaterialData* m_sharedMaterial		= nullptr;
	MaterialData* m_mutableMaterial		= nullptr;

private:
	MaterialData*		GetActiveMaterial();
	const MaterialData* GetActiveMaterial() const;
	void				TryAllocateMutableMaterial();



};