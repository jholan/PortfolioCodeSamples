#include "Engine/Rendering/ShaderUniformDescriptions.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include <d3dcompiler.h>
#include <d3d11shader.h>

#include "Engine/Core/EngineCommon.h"

#include "Engine/Rendering/Utility/RenderConstants.hpp"
#include "Engine/Rendering/ShaderProgram.h"

#define LIGHT_CONSTANT_BUFFER_BIND_POINT 7



const PropertyBufferDescription* PropertyDescription::GetContainingBuffer() const
{
	return m_containingBuffer;
}


const std::string& PropertyDescription::GetName() const
{
	return m_name;
}


uint PropertyDescription::GetOffsetIntoContainingBufferInBytes() const
{
	return m_containingBufferOffsetInBytes;
}


uint PropertyDescription::GetSizeInBytes() const
{
	return m_sizeInBytes;
}


uint PropertyDescription::GetElementCount() const
{
	return m_count;
}


const PropertyDescription* PropertyBufferDescription::GetPropertyDescription(const std::string& name) const
{
	const PropertyDescription* propertyDescription = nullptr;

	for (int i = 0; i < (int)m_propertyDescriptions.size(); ++i)
	{
		if (m_propertyDescriptions[i].GetName() == name)
		{
			propertyDescription = &(m_propertyDescriptions[i]);
			break;
		}
	}

	return propertyDescription;
}


const std::string& PropertyBufferDescription::GetName() const
{
	return m_name;
}


uint PropertyBufferDescription::GetBindPoint() const
{
	return m_bindPoint;
}


uint PropertyBufferDescription::GetSizeInBytes() const
{
	return m_bufferSizeInBytes;
}


uint GetSizeOfPropertyTypeBytes(_D3D_SHADER_VARIABLE_TYPE type)
{
	uint sizeBytes = 0;

	switch(type)
	{
	case D3D_SVT_INT:		// Fall through
	case D3D_SVT_FLOAT:		// Fall through
	case D3D_SVT_UINT:
	{
		sizeBytes = 4;
		break;
	}
	default:
	{
		GUARANTEE_OR_DIE(false, "Unsupported property type");
	}
	break;
	}

	return sizeBytes;
}


void PropertyBufferDescription::Fill(void* rd, const std::string& bufferName)
{
	// Grab the reflection data -----------------------------------------------------------------------
	ID3D11ShaderReflection* fragmentShaderReflectionData = (ID3D11ShaderReflection*)rd;

	ID3D11ShaderReflectionConstantBuffer* bufferData = fragmentShaderReflectionData->GetConstantBufferByName(bufferName.c_str());
	D3D11_SHADER_BUFFER_DESC sbd;
	HRESULT hr = bufferData->GetDesc(&sbd);

	D3D11_SHADER_INPUT_BIND_DESC rbd;
	hr = fragmentShaderReflectionData->GetResourceBindingDescByName(bufferName.c_str(), &rbd);



	// Buffer Information -----------------------------------------------------------------------------
	// Name
	m_name = bufferName;

	// Bind point
	m_bindPoint = rbd.BindPoint;

	// Buffer size
	m_bufferSizeInBytes = sbd.Size;



	// Information for each Property ------------------------------------------------------------------
	int numPropertiesInBuffer = sbd.Variables;

	// SHORT CIRCUIT. This is an empty block so lets skip it
	if (numPropertiesInBuffer <= 0)
	{
		return;
	}

	// For each property in the property buffer
	for (int i = 0; i < numPropertiesInBuffer; ++i)
	{
		ID3D11ShaderReflectionVariable* prop = bufferData->GetVariableByIndex(i);
		D3D11_SHADER_VARIABLE_DESC propDesc;
		prop->GetDesc(&propDesc);

		// Add to our list of properties
		m_propertyDescriptions.push_back(PropertyDescription());
		PropertyDescription* propertyDescription = &(m_propertyDescriptions[m_propertyDescriptions.size() - 1]);

		// Owner
		propertyDescription->m_containingBuffer = this;

		// Name
		propertyDescription->m_name = propDesc.Name;

		// Offset
		propertyDescription->m_containingBufferOffsetInBytes = propDesc.StartOffset;

		// Size
		propertyDescription->m_sizeInBytes = propDesc.Size;

		// Count (array length)
		ID3D11ShaderReflectionType* propTypeData = prop->GetType();
		D3D11_SHADER_TYPE_DESC propTypeDesc;
		propTypeData->GetDesc(&propTypeDesc);
		propertyDescription->m_count = propertyDescription->m_sizeInBytes / GetSizeOfPropertyTypeBytes(propTypeDesc.Type);
	}
}


void ShaderMaterialPropertyDescription::FillFromShaderProgram(const ShaderProgram* shaderProgram)
{
	Reset();

	// Reflect shader info
	ID3D11ShaderReflection* fragmentShaderReflectionData = NULL;
	D3DReflect(shaderProgram->GetFragmentShaderStage().GetBytecode(), shaderProgram->GetFragmentShaderStage().GetBytecodeSizeBytes(), IID_ID3D11ShaderReflection, (void**)&fragmentShaderReflectionData);


	// Get shader info
	D3D11_SHADER_DESC fragmentShaderDescription;
  	fragmentShaderReflectionData->GetDesc( &fragmentShaderDescription );


	// Get the initial number of property buffers
	uint numConstantBuffers = fragmentShaderDescription.ConstantBuffers;	// All constant buffers
	std::vector<std::string> propertyBufferNames;
	for (int i = 0; i < (int)numConstantBuffers; ++i)
	{
		// Get the constant buffers info
		ID3D11ShaderReflectionConstantBuffer* bufferData = fragmentShaderReflectionData->GetConstantBufferByIndex(i);
		D3D11_SHADER_BUFFER_DESC bufferDescription;
		HRESULT hr = bufferData->GetDesc(&bufferDescription);

		// We only reflect constant buffers so if we are a different type of buffer skip it.
		if (bufferDescription.Type != D3D_CT_CBUFFER)
		{
			continue;
		}

		// Get the bind point of the constant buffer
		D3D11_SHADER_INPUT_BIND_DESC rbd;
		hr = fragmentShaderReflectionData->GetResourceBindingDescByName(bufferDescription.Name, &rbd);

		if (rbd.BindPoint >= USER_CONSTANT_BUFFER_BIND_POINT_0)
		{
			propertyBufferNames.push_back(bufferDescription.Name);
		}


		// This is temporary 
		if (rbd.BindPoint == LIGHT_CONSTANT_BUFFER_BIND_POINT)
		{
			m_isLit = true;
		}
	}


	// Fill the data for each property buffer
	m_propertyBufferDescriptions.reserve(propertyBufferNames.size());
	for (int i = 0; i < (int)propertyBufferNames.size(); ++i)
	{
		const std::string& bufferName = propertyBufferNames[i];

		m_propertyBufferDescriptions.push_back(PropertyBufferDescription());
		PropertyBufferDescription* bufferDescription = &(m_propertyBufferDescriptions[m_propertyBufferDescriptions.size() - 1]);
		bufferDescription->Fill(fragmentShaderReflectionData, bufferName);
	}


	// Cleanup
	fragmentShaderReflectionData->Release();
}


void ShaderMaterialPropertyDescription::Reset()
{
	m_propertyBufferDescriptions.clear();
}


const PropertyBufferDescription* ShaderMaterialPropertyDescription::GetBufferDescription(const std::string& bufferName) const
{
	const PropertyBufferDescription* propertyBufferDescription = nullptr;

	for (int i = 0; i < (int)m_propertyBufferDescriptions.size(); ++i)
	{
		if (m_propertyBufferDescriptions[i].GetName() == bufferName)
		{
			propertyBufferDescription = &(m_propertyBufferDescriptions[i]);
			break;
		}
	}

	return propertyBufferDescription;
}


const PropertyBufferDescription* ShaderMaterialPropertyDescription::GetContainingBuffer(const std::string& propertyName) const
{
	const PropertyBufferDescription* propertyBufferDescription = nullptr;

	for (int i = 0; i < (int)m_propertyBufferDescriptions.size(); ++i)
	{
		const PropertyDescription* propertyDescription = m_propertyBufferDescriptions[i].GetPropertyDescription(propertyName);
		if (propertyDescription != nullptr)
		{
			propertyBufferDescription = &(m_propertyBufferDescriptions[i]);
			break;
		}
	}

	return propertyBufferDescription;
}


const PropertyDescription* ShaderMaterialPropertyDescription::GetPropertyDescription(const std::string& propertyName) const
{
	const PropertyDescription* propertyDescription = nullptr;

	for (int i = 0; i < (int)m_propertyBufferDescriptions.size(); ++i)
	{
		const PropertyDescription* testPropertyDescription = m_propertyBufferDescriptions[i].GetPropertyDescription(propertyName);
		if (testPropertyDescription != nullptr)
		{
			propertyDescription = testPropertyDescription;
			break;
		}
	}

	return propertyDescription;
}


bool ShaderMaterialPropertyDescription::IsLit() const
{
	return m_isLit;
}