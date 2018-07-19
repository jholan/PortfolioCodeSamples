#pragma once

#include <vector>

class ShaderProgram;

class PropertyDescription;
class PropertyBlockDescription;
class ShaderMaterialPropertyDescription;



class PropertyDescription
{
	friend class PropertyBlockDescription;
	friend class ShaderMaterialPropertyDescription;

public:
	const PropertyBlockDescription*	GetContainingBuffer() const;

	const std::string&				GetName() const;
	unsigned int					GetOffsetIntoContainingBufferInBytes() const;
	unsigned int					GetSizeInBytes() const;
	unsigned int					GetElementCount() const;


private:
	PropertyBlockDescription*		m_containingBuffer;		// My owner

	std::string						m_name;
	unsigned int					m_containingBufferOffsetInBytes;
	unsigned int					m_sizeInBytes;						// sizeof(Type) * count
	unsigned int					m_count;							// array if > 1
};



class PropertyBlockDescription
{
	friend class PropertyDescription;
	friend class ShaderMaterialPropertyDescription;

public:
	const PropertyDescription*		 GetPropertyDescription(const std::string& name) const;
									 
	const std::string&				 GetName() const;
	unsigned int					 GetBindPoint() const;
	unsigned int					 GetSizeInBytes() const;
									 
									 
private:							 
	void							 Fill(unsigned int programHandle, int index, int bindPoint);

	std::string						 m_name;
	unsigned int					 m_bindPoint;
	std::vector<PropertyDescription> m_propertyDescriptions;
	unsigned int					 m_bufferSizeInBytes;
};



class ShaderMaterialPropertyDescription
{
	friend class PropertyDescription;
	friend class PropertyBlockDescription;

public:
	const PropertyBlockDescription*			GetBufferDescription(const std::string& bufferName) const;
	const PropertyBlockDescription*			GetContainingBuffer(const std::string& propertyName) const;
	const PropertyDescription*				GetPropertyDescription(const std::string& propertyName) const;

	bool									IsLit() const;

	void									FillFromShaderProgram(const ShaderProgram* shaderProgram);



private:
	void									Reset();

	bool									m_isLit = false;
	std::vector<PropertyBlockDescription>	m_propertyBufferDescriptions;
};