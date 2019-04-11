#pragma once

#include <vector>

#include "Engine/Core/Types.hpp"

class ShaderProgram;

class PropertyDescription;
class PropertyBufferDescription;
class ShaderMaterialPropertyDescription;
 


class PropertyDescription
{
	friend class PropertyBufferDescription;
	friend class ShaderMaterialPropertyDescription;

public:
	const PropertyBufferDescription*	GetContainingBuffer() const;

	const std::string&					GetName() const;
	uint								GetOffsetIntoContainingBufferInBytes() const;
	uint								GetSizeInBytes() const;
	uint								GetElementCount() const;


private:
	PropertyBufferDescription*			m_containingBuffer;		// My owner

	std::string							m_name;
	uint								m_containingBufferOffsetInBytes;
	uint								m_sizeInBytes;						// sizeof(Type) * count
	uint								m_count;							// array if > 1
};



class PropertyBufferDescription
{
	friend class PropertyDescription;
	friend class ShaderMaterialPropertyDescription;

public:
	const PropertyDescription*	GetPropertyDescription(const std::string& name) const;

	const std::string&			GetName() const;
	uint						GetBindPoint() const;
	uint						GetSizeInBytes() const;


private:							 
	void								Fill(void* fragmentShaderReflectionData, const std::string& bufferName);

	std::string							m_name;
	uint								m_bindPoint;
	std::vector<PropertyDescription>	m_propertyDescriptions;
	uint								m_bufferSizeInBytes;
};



class ShaderMaterialPropertyDescription
{
	friend class PropertyDescription;
	friend class PropertyBufferDescription;

public:
	const PropertyBufferDescription*		GetBufferDescription(const std::string& bufferName) const;
	const PropertyBufferDescription*		GetContainingBuffer(const std::string& propertyName) const;
	const PropertyDescription*				GetPropertyDescription(const std::string& propertyName) const;

	bool									IsLit() const;

	void									FillFromShaderProgram(const ShaderProgram* shaderProgram);



private:
	void									Reset();

	bool									m_isLit = false;
	std::vector<PropertyBufferDescription>	m_propertyBufferDescriptions;
};