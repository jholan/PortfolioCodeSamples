#include "Engine/Rendering/ShaderUniformDescriptions.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Rendering/GLFunctions.h"
#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/Renderer.hpp"
extern Renderer* g_theRenderer;



unsigned int GetSizeOfGlType(GLint type)
{
	unsigned int size = 0;

	const unsigned int sizeOfFloat	= sizeof(float);
	const unsigned int sizeOfInt	= sizeof(int);
	const unsigned int sizeOfUInt	= sizeof(unsigned int);

	switch(type)
	{
	// ----------------------------------------
	case GL_FLOAT:
	{
		size = sizeOfFloat * 1;
		break;
	}

	case GL_FLOAT_VEC2:
	{
		size = sizeOfFloat * 2;
		break;
	}

	case GL_FLOAT_VEC3:
	{
		size = sizeOfFloat * 3;
		break;
	}

	case GL_FLOAT_VEC4:
	{
		size = sizeOfFloat * 4;
		break;
	}


	// -----------------------------------------
	case GL_INT:
	{
		size = sizeOfInt * 1;
		break;
	}

	case GL_INT_VEC2:
	{
		size = sizeOfInt * 2;
		break;
	}

	case GL_INT_VEC3:
	{
		size = sizeOfInt * 3;
		break;
	}

	case GL_INT_VEC4:
	{
		size = sizeOfInt * 4;
		break;
	}


	// -----------------------------------------
	case GL_UNSIGNED_INT:
	{
		size = sizeOfUInt * 1;
		break;
	}

	case GL_UNSIGNED_INT_VEC2:
	{
		size = sizeOfUInt * 2;
		break;
	}

	case GL_UNSIGNED_INT_VEC3:
	{
		size = sizeOfUInt * 3;
		break;
	}

	case GL_UNSIGNED_INT_VEC4:
	{
		size = sizeOfUInt * 4;
		break;
	}


	// -----------------------------------------
	case GL_FLOAT_MAT2:
	{
		size = (sizeOfFloat * 2) * 2;
		break;
	}

	case GL_FLOAT_MAT3:
	{
		size = (sizeOfFloat * 3) * 3;
		break;
	}

	case GL_FLOAT_MAT4:
	{
		size = (sizeOfFloat * 4) * 4;
		break;
	}

	case GL_FLOAT_MAT2x3:
	{
		size = (sizeOfFloat * 2) * 3;
		break;
	}

	case GL_FLOAT_MAT2x4:
	{
		size = (sizeOfFloat * 2) * 4;
		break;
	}

	case GL_FLOAT_MAT3x2:
	{
		size = (sizeOfFloat * 3) * 2;
		break;
	}

	case GL_FLOAT_MAT3x4:
	{
		size = (sizeOfFloat * 3) * 4;
		break;
	}
	
	case GL_FLOAT_MAT4x2:
	{
		size = (sizeOfFloat * 4) * 2;
		break;
	}

	case GL_FLOAT_MAT4x3:
	{
		size = (sizeOfFloat * 4) * 3;
		break;
	}


	// -----------------------------------------
	default:
		GUARANTEE_OR_DIE(false, "Unable to parse GL Type in PropertyBuffer");
		break;
	}

	return size;
}



// ---------------------------------------------------------------------------------------------------------------------------------------
// -- PropertyDescription ----------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------
const PropertyBlockDescription* PropertyDescription::GetContainingBuffer() const
{
	return m_containingBuffer;
}



const std::string& PropertyDescription::GetName() const
{
	return m_name;
}



unsigned int PropertyDescription::GetOffsetIntoContainingBufferInBytes() const
{
	return m_containingBufferOffsetInBytes;
}



unsigned int PropertyDescription::GetSizeInBytes() const
{
	return m_sizeInBytes;
}



unsigned int PropertyDescription::GetElementCount() const
{
	return m_count;
}



// ---------------------------------------------------------------------------------------------------------------------------------------
// -- PropertyBlockDescription -----------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------
const PropertyDescription* PropertyBlockDescription::GetPropertyDescription(const std::string& name) const
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



const std::string& PropertyBlockDescription::GetName() const
{
	return m_name;
}



unsigned int PropertyBlockDescription::GetBindPoint() const
{
	return m_bindPoint;
}



unsigned int PropertyBlockDescription::GetSizeInBytes() const
{
	return m_bufferSizeInBytes;
}



void PropertyBlockDescription::Fill(unsigned int programHandle, int index, int bindPoint)
{
	// Buffer Information -----------------------------------------------------------------------------
	GLsizei const	maxNameLength = 64; 
	GLint			nameLength	  = 0;
	char			name[maxNameLength];

	// Name
	glGetActiveUniformBlockName(programHandle, index, maxNameLength, &nameLength, name);
	m_name = name;

	// Bind point
	m_bindPoint = bindPoint;

	// Buffer size
	GLint bufferSizeInBytes = 0;
	glGetActiveUniformBlockiv(programHandle, index, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSizeInBytes);
	m_bufferSizeInBytes = bufferSizeInBytes;



	// Information for each Property ------------------------------------------------------------------
	GLint numPropertiesInBuffer;
	glGetActiveUniformBlockiv(programHandle, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numPropertiesInBuffer);
	
	// Short Circuit. This is an empty block so lets skip it
	if (numPropertiesInBuffer <= 0)
	{
		return;
	}


	// Get each properties index into the buffer so we can query it
	GLint* indices = STACK_ALLOC_ARRAY(GLint, numPropertiesInBuffer);
	glGetActiveUniformBlockiv(programHandle, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);

	// Get the size, offset, and GL type of each property
	GLint* offsetsIntoBufferInBytes = STACK_ALLOC_ARRAY(GLint, numPropertiesInBuffer);
	GLint* glTypes					= STACK_ALLOC_ARRAY(GLint, numPropertiesInBuffer);
	GLint* numElements				= STACK_ALLOC_ARRAY(GLint, numPropertiesInBuffer);

	glGetActiveUniformsiv(programHandle, numPropertiesInBuffer, (GLuint*)indices, GL_UNIFORM_OFFSET, offsetsIntoBufferInBytes);
	glGetActiveUniformsiv(programHandle, numPropertiesInBuffer, (GLuint*)indices, GL_UNIFORM_TYPE,	 glTypes);
	glGetActiveUniformsiv(programHandle, numPropertiesInBuffer, (GLuint*)indices, GL_UNIFORM_SIZE,	 numElements);


	m_propertyDescriptions.reserve(numPropertiesInBuffer);
	for (GLint i = 0; i < numPropertiesInBuffer; ++i)
	{
		m_propertyDescriptions.push_back(PropertyDescription());
		PropertyDescription* propertyDescription = &(m_propertyDescriptions[m_propertyDescriptions.size() - 1]);

		// Owner
		propertyDescription->m_containingBuffer = this;

		// Name
		glGetActiveUniformName(programHandle, (GLuint)indices[i], maxNameLength, &nameLength, name);
		propertyDescription->m_name = name;

		// Offset
		propertyDescription->m_containingBufferOffsetInBytes = offsetsIntoBufferInBytes[i];

		// Size
		propertyDescription->m_sizeInBytes = GetSizeOfGlType(glTypes[i]) * numElements[i];

		// Count
		propertyDescription->m_count = numElements[i];
	}
}



// ---------------------------------------------------------------------------------------------------------------------------------------
// -- ShaderMaterialPropertyDescription --------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------
void ShaderMaterialPropertyDescription::FillFromShaderProgram(const ShaderProgram* shaderProgram)
{
	Reset();

	g_theRenderer->BindShaderProgram(shaderProgram);

	GLuint programHandle = (GLuint)shaderProgram->GetProgramHandle();
	GLint numPropertyBuffers;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORM_BLOCKS, &numPropertyBuffers);

	m_propertyBufferDescriptions.reserve(numPropertyBuffers);
	for (GLint i = 0; i < numPropertyBuffers; ++i)
	{
		GLint bindPoint = -1;
		glGetActiveUniformBlockiv(programHandle, i, GL_UNIFORM_BLOCK_BINDING, &bindPoint);
		if (bindPoint >= (int)Renderer::MATERIAL_PROPERTY_BUFFER_0_BIND_POINT)
		{
			m_propertyBufferDescriptions.push_back(PropertyBlockDescription());
			PropertyBlockDescription* bufferDescription = &(m_propertyBufferDescriptions[m_propertyBufferDescriptions.size() - 1]);
			bufferDescription->Fill(programHandle, i, bindPoint);
		}
		else if ((unsigned int)bindPoint == Renderer::LIGHT_BUFFER_BIND_POINT)
		{
			m_isLit = true;
		}

	}
}



void ShaderMaterialPropertyDescription::Reset()
{
	m_propertyBufferDescriptions.clear();
}



const PropertyBlockDescription* ShaderMaterialPropertyDescription::GetBufferDescription(const std::string& bufferName) const
{
	const PropertyBlockDescription* propertyBufferDescription = nullptr;

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



const PropertyBlockDescription* ShaderMaterialPropertyDescription::GetContainingBuffer(const std::string& propertyName) const
{
	const PropertyBlockDescription* propertyBufferDescription = nullptr;

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
