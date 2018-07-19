#include "Engine/Rendering/GLFunctions.h"



void GLCheckError( char const *file, int line )
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		DebuggerPrintf( "GL ERROR [0x%04x] at [%s(%i)]\n", error, file, line );
	}
}



PFNWGLGETEXTENSIONSSTRINGARBPROC  wglGetExtensionsStringARB  = nullptr;
PFNWGLCHOOSEPIXELFORMATARBPROC	  wglChoosePixelFormatARB	 = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

PFNGLVIEWPORTPROC			glViewport				= nullptr;

PFNGLCLEARPROC				glClear					= nullptr;
PFNGLCLEARCOLORPROC			glClearColor			= nullptr;

PFNGLENABLEPROC				glEnable				= nullptr;
PFNGLDISABLEPROC			glDisable				= nullptr;

PFNGLLINEWIDTHPROC			glLineWidth				= nullptr;

PFNGLBLENDFUNCPROC			glBlendFunc				= nullptr;

PFNGLCREATESHADERPROC		glCreateShader			= nullptr;
PFNGLDELETESHADERPROC		glDeleteShader			= nullptr;
PFNGLSHADERSOURCEPROC		glShaderSource			= nullptr;
PFNGLCOMPILESHADERPROC		glCompileShader			= nullptr;
PFNGLGETSHADERIVPROC		glGetShaderiv			= nullptr;
PFNGLGETSHADERINFOLOGPROC	glGetShaderInfoLog		= nullptr;
PFNGLATTACHSHADERPROC		glAttachShader			= nullptr;
PFNGLDETACHSHADERPROC		glDetachShader			= nullptr;

PFNGLCREATEPROGRAMPROC		glCreateProgram			= nullptr;
PFNGLDELETEPROGRAMPROC		glDeleteProgram			= nullptr;
PFNGLLINKPROGRAMPROC		glLinkProgram			= nullptr;
PFNGLGETPROGRAMIVPROC		glGetProgramiv			= nullptr;
PFNGLGETPROGRAMINFOLOGPROC	glGetProgramInfoLog		= nullptr;
PFNGLUSEPROGRAMPROC			glUseProgram			= nullptr;

PFNGLGENBUFFERSPROC			glGenBuffers			= nullptr;
PFNGLDELETEBUFFERSPROC		glDeleteBuffers			= nullptr;
PFNGLBINDBUFFERPROC			glBindBuffer			= nullptr;
PFNGLBUFFERDATAPROC			glBufferData			= nullptr;

PFNGLGENVERTEXARRAYSPROC	glGenVertexArrays		= nullptr;
PFNGLDELETEVERTEXARRAYSPROC	glDeleteVertexArrays	= nullptr;
PFNGLBINDVERTEXARRAYPROC	glBindVertexArray		= nullptr;

PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation			= nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray	= nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray	= nullptr;
PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer		= nullptr;

PFNGLDRAWARRAYSPROC			glDrawArrays = nullptr;
PFNGLDRAWELEMENTSPROC		glDrawElements = nullptr;

PFNGLGETUNIFORMLOCATIONPROC	glGetUniformLocation	= nullptr;
PFNGLUNIFORMMATRIX4FVPROC	glUniformMatrix4fv		= nullptr;
PFNGLUNIFORM1IPROC			glUniform1i				= nullptr;
PFNGLUNIFORM1UIPROC			glUniform1ui			= nullptr;
PFNGLUNIFORM1FPROC			glUniform1f				= nullptr;
PFNGLUNIFORM2FVPROC			glUniform2fv			= nullptr;
PFNGLUNIFORM3FVPROC			glUniform3fv			= nullptr;
PFNGLUNIFORM4FVPROC			glUniform4fv			= nullptr;

PFNGLGENSAMPLERSPROC		 glGenSamplers		  = nullptr;
PFNGLDELETESAMPLERSPROC		 glDeleteSamplers	  = nullptr;
PFNGLSAMPLERPARAMETERIPROC   glSamplerParameteri  = nullptr;
PFNGLSAMPLERPARAMETERFVPROC	 glSamplerParameterfv = nullptr;
PFNGLBINDSAMPLERPROC		 glBindSampler		  = nullptr;

PFNGLBINDTEXTUREPROC		 glBindTexture		 = nullptr;
PFNGLACTIVETEXTUREPROC		 glActiveTexture	 = nullptr;
PFNGLPIXELSTOREIPROC		 glPixelStorei		 = nullptr;
PFNGLGENTEXTURESPROC		 glGenTextures		 = nullptr;
PFNGLTEXPARAMETERIPROC		 glTexParameteri	 = nullptr;
PFNGLTEXIMAGE2DPROC			 glTexImage2D		 = nullptr;
PFNGLCOPYIMAGESUBDATAPROC	 glCopyImageSubData	 = nullptr;
PFNGLTEXSTORAGE2DPROC		 glTexStorage2D		 = nullptr;
PFNGLTEXSUBIMAGE2DPROC		 glTexSubImage2D	 = nullptr;

PFNGLGENERATEMIPMAPPROC		glGenerateMipmap	 = nullptr;

PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers		 = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC			glDeleteFramebuffers	 = nullptr;
PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer		 = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC			glFramebufferTexture	 = nullptr;
PFNGLDRAWBUFFERSPROC				glDrawBuffers			 = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC		glCheckFramebufferStatus = nullptr;
PFNGLBLITFRAMEBUFFERPROC			glBlitFramebuffer		 = nullptr;

PFNGLDEPTHFUNCPROC			glDepthFunc	  = nullptr;
PFNGLDEPTHMASKPROC			glDepthMask	  = nullptr;
PFNGLCLEARDEPTHFPROC		glClearDepthf = nullptr;

PFNGLGETTEXIMAGEPROC		glGetTexImage = nullptr;

PFNGLGETERRORPROC			glGetError = nullptr;

PFNGLPOLYGONMODEPROC		glPolygonMode = nullptr;

PFNGLCULLFACEPROC			glCullFace = nullptr;

PFNGLFRONTFACEPROC			glFrontFace = nullptr;

PFNGLGETBOOLEANVPROC		glGetBooleanv = nullptr;

PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate  = nullptr;
PFNGLBLENDFUNCSEPARATEPROC	   glBlendFuncSeparate		= nullptr;

PFNGLBINDBUFFERBASEPROC		glBindBufferBase = nullptr;

PFNGLGETACTIVEUNIFORMBLOCKIVPROC	glGetActiveUniformBlockiv	= nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC	glGetActiveUniformBlockName = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC		glGetActiveUniformsiv		= nullptr;
PFNGLGETACTIVEUNIFORMNAMEPROC		glGetActiveUniformName		= nullptr;
