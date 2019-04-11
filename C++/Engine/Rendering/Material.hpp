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
class Texture2D;
class ShaderResourceView;
class Sampler;
class PropertyBlock;
class PropertyBufferDescription;



class MaterialData
{
public:
	MaterialData();
	MaterialData(const MaterialData& toCopy);
	MaterialData(const XMLEle& definition);
	~MaterialData();
	MaterialData* Clone() const;
	MaterialData& operator=( const MaterialData& toCopy);


	// Shader
	void						SetShader(const std::string& name);
	void						SetShader(const Shader* shader);
	const Shader*				GetShader() const;


	// Textures
	void						SetTexture(unsigned int bindPoint, const std::string& name);
	void						SetTexture(unsigned int bindPoint, const Texture2D* texture);
					
	bool						IsValidTexture( unsigned int bindPoint) const;
	const ShaderResourceView*	GetTexture(unsigned int bindPoint) const;
	unsigned int				GetNumberOfTextures() const;


	// Samplers
	void						SetSampler(unsigned int bindPoint, const Sampler* sampler);

	bool						IsValidSampler( unsigned int bindPoint) const;
	const Sampler*				GetSampler(unsigned int bindPoint) const;
	unsigned int				GetNumberOfSamplers() const;
	

	// Properties
	void						SetProperty(const std::string& property, int			value);
	void						SetProperty(const std::string& property, unsigned int   value);
	void						SetProperty(const std::string& property, float			value);
	void						SetProperty(const std::string& property, const Vector2& value);
	void						SetProperty(const std::string& property, const Vector3& value);
	void						SetProperty(const std::string& property, const Vector4& value);
	void						SetProperty(const std::string& property, const Matrix4& value);
	void						SetProperty(const std::string& property, const RGBA&    value);
	void						SetProperty(const std::string& property, const void*	data,  unsigned int dataSize);
	void						BindPropertyBuffers() const;



	// Statics
	static MaterialData*		Get(const std::string& name);
	static MaterialData*		FromShader(const std::string& shaderName);

	static void					RegisterMaterialData(const std::string& name, MaterialData* material, bool update = false);
	static void					ParseMaterialDatas(const std::string& materialFile);



private:
	void						TryInitializeStatics();
	PropertyBlock*				CreateOrGetPropertyBuffer(const PropertyBufferDescription* description);

	// Textures
	void						EnsureTextureArrayCanHoldXTextures(unsigned int count);
	void						AddSRVToBindPoint(unsigned int bindPoint, const Texture2D* texture);
	void						RemoveSRVFromBindPoint(unsigned int bindPoint);

	void						EnsureSamplerArrayCanHoldXSamplers(unsigned int count);

	const Shader*								m_shader;
	std::vector<ShaderResourceView*>			m_textureViews;
	std::vector<const Sampler*>					m_samplers;
	std::vector<PropertyBlock*>					m_propertyBuffers;


	static std::map<std::string, MaterialData*> s_loadedMaterialDatas;


	static MaterialData*						DEFAULT_MATERIAL;
};



class Material
{
public:
	Material();
	~Material();


	// Shader
	void						SetShader(const std::string& name);
	void						SetShader(const Shader* shader);
	const Shader*				GetShader() const;


	// Textures
	void						SetTexture(unsigned int bindPoint, const std::string& name);
	void						SetTexture(unsigned int bindPoint, const Texture2D* texture);

	bool						IsValidTexture(unsigned int bindPoint) const;
	const ShaderResourceView*	GetTexture(unsigned int bindPoint) const;
	unsigned int				GetNumberOfTextures() const;


	// Samplers
	void						SetSampler(unsigned int bindPoint, const Sampler* sampler);

	bool						IsValidSampler( unsigned int bindPoint) const;
	const Sampler*				GetSampler(unsigned int bindPoint) const;
	unsigned int				GetNumberOfSamplers() const;


	// Properties
	void						SetProperty(const std::string& property, int			value);
	void						SetProperty(const std::string& property, unsigned int   value);
	void						SetProperty(const std::string& property, float			value);
	void						SetProperty(const std::string& property, const Vector2& value);
	void						SetProperty(const std::string& property, const Vector3& value);
	void						SetProperty(const std::string& property, const Vector4& value);
	void						SetProperty(const std::string& property, const Matrix4& value);
	void						SetProperty(const std::string& property, const RGBA&    value);
	void						BindPropertyBuffers() const;


	// Statics
	static Material* Get(const std::string& name);
	static Material* FromShader(const std::string& shaderName); // Automatically mutable

	static void		 RegisterMaterial(const std::string& name, Material* material, bool update = false);
	static void		 ParseMaterials(const std::string& materialFile);



private:
	MaterialData* m_sharedMaterial		= nullptr;
	MaterialData* m_mutableMaterial		= nullptr;

	MaterialData*		GetActiveMaterial();
	const MaterialData* GetActiveMaterial() const;
	void				TryAllocateMutableMaterial();
};