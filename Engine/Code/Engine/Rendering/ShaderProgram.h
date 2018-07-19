#pragma once

#include <map>
#include <string>

#include "Engine/Rendering/ShaderUniformDescriptions.hpp"



class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	bool LoadFromFiles(const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& defines = "");
	bool LoadFromFiles_Interal(const std::string& vertexFilepath, const std::string& fragmentFilepath, bool record = true, const std::string& defines = "");
	bool LoadFromStrings(const std::string& vertexShader, const std::string& fragmentShader);
	bool LoadFromStrings_Internal(const std::string& vertexShader, const std::string& fragmentShader, bool record = true);

	unsigned int		GetProgramHandle() const;
	bool				WasLoadedFromStrings() const;
	const std::string&	GetVertexShaderFilepath() const;
	const std::string&	GetFragmentShaderFilepath() const;
	void				Reload();
	void				ReloadAs(const std::string& name, const std::string& defines = "");
	bool				IsLit() const;
	const ShaderMaterialPropertyDescription* GetShaderMaterialDescription() const;

	static const ShaderProgram*	  CreateOrGet(const std::string& name, const std::string& defines = "");
	static ShaderProgram*		  CreateOrGetMutable(const std::string& name, const std::string& defines = "");
	static const ShaderProgram*   CreateOrGet(const std::string& name, const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& defines = "");
	static ShaderProgram*		  CreateOrGetMutable(const std::string& name, const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& defines = "");
	static const ShaderProgram*	  CreateFromStrings(const std::string& name, const std::string& vertexShader, const std::string& fragmentShader);
	static void					  ReloadAllShaderPrograms();

private:
	bool		m_fromStrings			= false;
	std::string m_vertexShaderFilepath;
	std::string m_fragmentShaderFilepath;
	std::string m_defines;

	bool		m_isLit = false;

	unsigned int m_programHandle;

	ShaderMaterialPropertyDescription m_materialPropertiesDescription;


	static std::map<std::string, ShaderProgram*> s_loadedPrograms;

	static const std::string INVALID_VS;
	static const std::string INVALID_FS;
};