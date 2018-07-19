#include "Engine/Rendering/ShaderProgram.h"

#include <stdio.h>
#include <direct.h>

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.h"

#include "Engine/Rendering/GLFunctions.h"



ShaderProgram::ShaderProgram()
	: m_fromStrings(false)
	, m_vertexShaderFilepath("")
	, m_fragmentShaderFilepath("")
	, m_programHandle(NULL)
{
}


ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(m_programHandle);
	m_programHandle = NULL;

	m_vertexShaderFilepath = "";
	m_fragmentShaderFilepath = "";

	m_fromStrings = false;
}

void* FileReadToNewBuffer( char const *filename )
{
	FILE *fp = nullptr;
	fopen_s( &fp, filename, "r" );

	if (fp == nullptr) {
		return nullptr;
	}

	size_t size = 0U;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	unsigned char *buffer = (unsigned char*) malloc(size + 1U); // space for NULL

	size_t read = fread( buffer, 1, size, fp );
	fclose(fp);

	buffer[read] = NULL;

	return buffer; 
}

static std::string FormatGLErrorString(const char* unformattedErrorMessage, unsigned int numDefines = 0)
{
	const char* copyStart = FindFirstCharacterInString(unformattedErrorMessage, "(");
	std::string errorMessage;
	if(copyStart == nullptr)
	{
		errorMessage = std::string(unformattedErrorMessage);
	}
	else
	{
		errorMessage = std::string(copyStart);
	}

	size_t spacePos = errorMessage.find_first_of(' ');
	if (spacePos != std::string::npos)
	{
		errorMessage.erase(errorMessage.begin() + spacePos);
	}

	// Update line number to account for defines
	size_t lineNumStart = errorMessage.find_first_of('(', 0);
	size_t lineNumEnd = errorMessage.find_first_of(')', lineNumStart);
	if (lineNumStart != std::string::npos && lineNumEnd != std::string::npos)
	{
		lineNumStart += 1;

		std::string lineNumberString = errorMessage.substr(lineNumStart, lineNumEnd - lineNumStart);
		int lineNumber = StringToInt(lineNumberString.c_str());

		lineNumber -= (int)numDefines;
		TODO("int, float, vec to string functions");
		lineNumberString = Stringf("%i", lineNumber);
		errorMessage.replace(lineNumStart, lineNumEnd - lineNumStart, lineNumberString);
	}

	return errorMessage;
}

static void LogShaderError(GLuint shader_id, const char* filename = nullptr, unsigned int numDefines = 0)
{
	// figure out how large the buffer needs to be
	GLint length;
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

	// make a buffer, and copy the log to it. 
	char* buffer = new char[length + 1];
	glGetShaderInfoLog(shader_id, length, &length, buffer);

	// Print it out (may want to do some additional formatting)
	buffer[length] = NULL;
	if (filename != nullptr)
	{
		// Get the current working directory:   
		char* currentWorkingDirectoryPath = nullptr;  
		if( (currentWorkingDirectoryPath = _getcwd( NULL, 0 )) != nullptr )  
		{
			DebuggerPrintf( "%s\\", currentWorkingDirectoryPath);  
			free(currentWorkingDirectoryPath);  
		}

		// Append the local filepath
		DebuggerPrintf("%s ", filename);
	}

	std::string errorMessage = FormatGLErrorString(buffer, numDefines);
	DebuggerPrintf( "%s", errorMessage.c_str() );

	// free up the memory we used. 
	delete buffer;
}


static GLuint LoadShaderFromString( const char* shaderContents, size_t shaderLength, GLenum type, const char* filename = nullptr, unsigned int numDefines = 0)
{
	// Create a shader
	GLuint shaderID = glCreateShader(type);
	GUARANTEE_OR_DIE(shaderID != NULL, "Could not create shader from file");

	// Bind source to it, and compile
	// You can add multiple strings to a shader – they will 
	// be concatenated together to form the actual source object.
	GLint shader_length = (GLint)shaderLength;
	glShaderSource(shaderID, 1, &shaderContents, &(GLint)shader_length);
	glCompileShader(shaderID);

	// Check status
	GLint status;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		LogShaderError(shaderID, filename, numDefines); // function we write
		//glDeleteShader(shaderID);
		//shaderID = NULL;
	}

	return shaderID;
}



static unsigned int InjectDefines(std::string& file, const std::string& defines)
{
	// Get insertionPoint
	size_t startPoint = file.find("#version");
	size_t insertionPoint = file.find('\n', startPoint) + 1;

	// Generate define strings
	std::vector<std::string> defStrings = ParseTokenStringToStrings(defines, ",");
	for (int i = 0; i < (int)defStrings.size(); ++i)
	{
		size_t equalsPoint = defStrings[i].find_first_of('=', 0);
		if (equalsPoint != std::string::npos)
		{
			defStrings[i].replace(equalsPoint, 1, " ");
		}
		std::string d = "#define " + defStrings[i] + '\n';

		// Inject define
		file.insert(insertionPoint, d);
		insertionPoint += d.size();
	}

	return defStrings.size();
}



static GLuint LoadShaderFromFile( const std::string& filename, const std::string& defines, GLenum type )
{
	char *src = (char*)FileReadToNewBuffer(filename.c_str());
	GUARANTEE_OR_DIE(src != nullptr, "File not loaded");

	std::string completeFile = src;
	unsigned int numDefines = InjectDefines(completeFile, defines);

	GLuint shaderID = LoadShaderFromString(completeFile.c_str(), completeFile.size(), type, filename.c_str(), numDefines);

	free(src);

	return shaderID;
}


static void LogProgramError(GLuint program_id)
{
	// get the buffer length
	GLint length;
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);

	// copy the log into a new buffer
	char *buffer = new char[length + 1];
	glGetProgramInfoLog(program_id, length, &length, buffer);

	// print it to the output pane
	buffer[length] = NULL;
	DebuggerPrintf("%s", buffer);
	//GUARANTEE_OR_DIE(0, "Error loading Program");           

	// cleanup
	delete buffer;
}


static GLuint CreateAndLinkProgram( GLuint programHandle, GLint vs, GLint fs )
{
	// create the program handle - how you will reference
	// this program within OpenGL, like a texture handle
	GLuint program_id = programHandle;
	if (program_id == 0)
	{
		program_id = glCreateProgram();
	}
	GUARANTEE_OR_DIE( program_id != 0 , "Could not create shader program");

	// Attach the shaders you want to use
	glAttachShader( program_id, vs );
	glAttachShader( program_id, fs );

	// Link the program (create the GPU program)
	glLinkProgram( program_id );

	// Check for link errors - usually a result
	// of incompatibility between stages.
	GLint link_status;
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE) {
		LogProgramError(program_id);
		glDeleteProgram(program_id);
		program_id = 0;
	} 

	// no longer need the shaders, you can detach them if you want
	// (not necessary)
	glDetachShader( program_id, vs );
	glDetachShader( program_id, fs );

	return program_id;
}


bool ShaderProgram::LoadFromFiles(const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& defines)
{
	return LoadFromFiles_Interal(vertexFilepath, fragmentFilepath, true, defines); 
}



bool ShaderProgram::LoadFromFiles_Interal(const std::string& vertexFilepath, const std::string& fragmentFilepath, bool record, const std::string& defines)
{
	// Compile the two stages we're using (all shaders will implement the vertex and fragment stages)
	// later on, we can add in more stages;
	GLuint vert_shader = LoadShaderFromFile( vertexFilepath, defines, GL_VERTEX_SHADER ); 
	GLuint frag_shader = LoadShaderFromFile( fragmentFilepath, defines, GL_FRAGMENT_SHADER ); 

	// Link the program
	// program_handle is a member GLuint. 
	m_programHandle = (unsigned int)CreateAndLinkProgram( (GLuint)m_programHandle, vert_shader, frag_shader ); 
	glDeleteShader( vert_shader ); 
	glDeleteShader( frag_shader ); 

	if (m_programHandle == NULL)
	{
		LoadFromStrings_Internal(INVALID_VS, INVALID_FS, false);
	}

	// at the bottom this overwrites the state set by LoadFromString WHICH WE WANT for reloading
	if (record == true)
	{
		m_fromStrings = false;
		m_vertexShaderFilepath = vertexFilepath;
		m_fragmentShaderFilepath = fragmentFilepath;
		m_defines = defines;

		m_materialPropertiesDescription.FillFromShaderProgram(this);
		m_isLit = m_materialPropertiesDescription.IsLit();
	}

	return (m_programHandle != NULL); 
}


bool ShaderProgram::LoadFromStrings(const std::string& vertexShader, const std::string& fragmentShader)
{
	return LoadFromStrings_Internal(vertexShader, fragmentShader); 
}


bool ShaderProgram::LoadFromStrings_Internal(const std::string& vertexShader, const std::string& fragmentShader, bool record)
{
	// Compile the two stages we're using (all shaders will implement the vertex and fragment stages)
	// later on, we can add in more stages;
	GLuint vert_shader = LoadShaderFromString( vertexShader.c_str(),	vertexShader.length(),	 GL_VERTEX_SHADER ); 
	GLuint frag_shader = LoadShaderFromString( fragmentShader.c_str(),	fragmentShader.length(), GL_FRAGMENT_SHADER ); 

	// Link the program
	// program_handle is a member GLuint. 
	m_programHandle = (unsigned int)CreateAndLinkProgram( (GLuint)m_programHandle, vert_shader, frag_shader ); 
	glDeleteShader( vert_shader ); 
	glDeleteShader( frag_shader ); 

	if (record == true)
	{
		m_fromStrings = true;
		m_vertexShaderFilepath = vertexShader;
		m_fragmentShaderFilepath = fragmentShader;
	}

	return (m_programHandle != NULL); 
}


unsigned int ShaderProgram::GetProgramHandle() const
{
	return m_programHandle;
}


bool ShaderProgram::WasLoadedFromStrings() const
{
	return m_fromStrings;
}


const std::string& ShaderProgram::GetVertexShaderFilepath() const
{
	return m_vertexShaderFilepath;
}


const std::string& ShaderProgram::GetFragmentShaderFilepath() const
{
	return m_fragmentShaderFilepath;
}


void ShaderProgram::Reload()
{
	if (!WasLoadedFromStrings())
	{
		LoadFromFiles(m_vertexShaderFilepath, m_fragmentShaderFilepath, m_defines);
	}
}

void ShaderProgram::ReloadAs(const std::string& name, const std::string& defines)
{
	if (!WasLoadedFromStrings())
	{
		auto alreadyExists = s_loadedPrograms.find(name);
		if (alreadyExists == s_loadedPrograms.end())
		{
			// No shader loaded with that name
			std::string vsFilepath = name + ".vs";
			std::string fsFilepath = name + ".fs";
			LoadFromFiles_Interal(vsFilepath, fsFilepath, false, defines);
		}
		else
		{
			const ShaderProgram* named = alreadyExists->second;
			LoadFromFiles_Interal(named->m_vertexShaderFilepath, named->m_fragmentShaderFilepath, false, defines);
		}
	}
}



bool ShaderProgram::IsLit() const
{
	return m_isLit;
}



const ShaderMaterialPropertyDescription* ShaderProgram::GetShaderMaterialDescription() const
{
	return &m_materialPropertiesDescription;
}



const ShaderProgram* ShaderProgram::CreateOrGet(const std::string& name, const std::string& defines)
{
	const ShaderProgram* shaderProgram = CreateOrGet(name, name + ".vs", name + ".fs", defines);
	return shaderProgram;
}



ShaderProgram* ShaderProgram::CreateOrGetMutable(const std::string& name, const std::string& defines)
{
	ShaderProgram* shaderProgram = CreateOrGetMutable(name, name + ".vs", name + ".fs", defines);
	return shaderProgram;
}



const ShaderProgram* ShaderProgram::CreateOrGet(const std::string& name, const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& defines)
{
	return CreateOrGetMutable(name, vertexFilepath, fragmentFilepath, defines);
}



ShaderProgram* ShaderProgram::CreateOrGetMutable(const std::string& name, const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& defines)
{
	ShaderProgram* shaderProgram = nullptr;

	std::map<std::string, ShaderProgram*>::iterator alreadyExists = s_loadedPrograms.find(name);
	if (alreadyExists == s_loadedPrograms.end())
	{
		// The ShaderProgram doesn't exist.
		shaderProgram = new ShaderProgram();
		bool loadedSuccessfully = shaderProgram->LoadFromFiles(vertexFilepath, fragmentFilepath, defines);
		GUARANTEE_OR_DIE(loadedSuccessfully, "Shader and Invalid shader could not be loaded");
		s_loadedPrograms[name] = shaderProgram;
	}
	else
	{
		// The ShaderProgram does exist.
		shaderProgram = s_loadedPrograms[name];
	}

	return shaderProgram;
}



const ShaderProgram* ShaderProgram::CreateFromStrings(const std::string& name, const std::string& vertexShader, const std::string& fragmentShader)
{
	ShaderProgram* shaderProgram = nullptr;

	std::map<std::string, ShaderProgram*>::iterator alreadyExists = s_loadedPrograms.find(name);
	if (alreadyExists == s_loadedPrograms.end())
	{
		// The ShaderProgram doesn't exist
		shaderProgram = new ShaderProgram();
		bool loadedSuccessfully = shaderProgram->LoadFromStrings(vertexShader, fragmentShader);
		GUARANTEE_OR_DIE(loadedSuccessfully, "Shader could not be loaded from strings");
		s_loadedPrograms[name] = shaderProgram;
	}
	else
	{
		// The ShaderProgram does exist.
		shaderProgram = s_loadedPrograms[name];
	}

	return shaderProgram;
}



void ShaderProgram::ReloadAllShaderPrograms()
{
	for (auto iter = s_loadedPrograms.begin(); iter != s_loadedPrograms.end(); ++iter)
	{
		ShaderProgram* shaderProgram = iter->second;
		shaderProgram->Reload();
	}
}



std::map<std::string, ShaderProgram*> ShaderProgram::s_loadedPrograms;

const std::string ShaderProgram::INVALID_VS =	"#version 420 core						\n"
												"in vec3 POSITION;						\n"
												"void main( void )						\n"
												"{										\n"
												"	gl_Position = vec4( POSITION, 1 );	\n"
												"}										\n";
const std::string ShaderProgram::INVALID_FS =	"#version 420 core						\n"
												"out vec4 outColor;						\n" 
												"void main( void )						\n"
												"{										\n"
												"	outColor = vec4( 1, 0, 1, 1 );		\n"
												"}										\n";