#pragma once

#include "ThirdParty/GL/glcorearb.h"
#include "ThirdParty/GL/glext.h"
#include "ThirdParty/GL/wglext.h"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"



// Use this to deduce type of the pointer so we can cast; 
template <typename T>
bool wglGetTypedProcAddress( T *out, char const *name ) 
{
	// Grab the function from the currently bound render context
	// most opengl 2.0+ features will be found here
	*out = (T) wglGetProcAddress(name); 

	if ((*out) == nullptr) {
		// if it is not part of wgl (the device), then attempt to get it from the GLL library
		// (most OpenGL functions come from here)
		*out = (T) GetProcAddress( gGLLibrary, name); 
	}

	return (*out != nullptr); 
}

#define GL_BIND_FUNCTION(f)      wglGetTypedProcAddress( &f, #f )

void GLCheckError( char const *file, int line );


extern PFNWGLGETEXTENSIONSSTRINGARBPROC  wglGetExtensionsStringARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC	 wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

extern PFNGLVIEWPORTPROC			glViewport;

extern PFNGLCLEARPROC				glClear;
extern PFNGLCLEARCOLORPROC			glClearColor;

extern PFNGLENABLEPROC				glEnable;
extern PFNGLDISABLEPROC				glDisable;

extern PFNGLLINEWIDTHPROC			glLineWidth;

extern PFNGLBLENDFUNCPROC			glBlendFunc;

extern PFNGLCREATESHADERPROC		glCreateShader;
extern PFNGLDELETESHADERPROC		glDeleteShader;
extern PFNGLSHADERSOURCEPROC		glShaderSource;
extern PFNGLCOMPILESHADERPROC		glCompileShader;
extern PFNGLGETSHADERIVPROC			glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC	glGetShaderInfoLog;
extern PFNGLATTACHSHADERPROC		glAttachShader;
extern PFNGLDETACHSHADERPROC		glDetachShader;

extern PFNGLCREATEPROGRAMPROC		glCreateProgram;
extern PFNGLDELETEPROGRAMPROC		glDeleteProgram;
extern PFNGLLINKPROGRAMPROC			glLinkProgram;
extern PFNGLGETPROGRAMIVPROC		glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC	glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC			glUseProgram;

extern PFNGLGENBUFFERSPROC			glGenBuffers;
extern PFNGLDELETEBUFFERSPROC		glDeleteBuffers;
extern PFNGLBINDBUFFERPROC			glBindBuffer;
extern PFNGLBUFFERDATAPROC			glBufferData;

extern PFNGLGENVERTEXARRAYSPROC		glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC	glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC		glBindVertexArray;

extern PFNGLGETATTRIBLOCATIONPROC		glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;

extern PFNGLDRAWARRAYSPROC			glDrawArrays;
extern PFNGLDRAWELEMENTSPROC		glDrawElements;

extern PFNGLGETUNIFORMLOCATIONPROC	glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC	glUniformMatrix4fv;
extern PFNGLUNIFORM1IPROC			glUniform1i;
extern PFNGLUNIFORM1UIPROC			glUniform1ui;
extern PFNGLUNIFORM1FPROC			glUniform1f;
extern PFNGLUNIFORM2FVPROC			glUniform2fv;
extern PFNGLUNIFORM3FVPROC			glUniform3fv;
extern PFNGLUNIFORM4FVPROC			glUniform4fv;

extern PFNGLGENSAMPLERSPROC			glGenSamplers;
extern PFNGLDELETESAMPLERSPROC		glDeleteSamplers;
extern PFNGLSAMPLERPARAMETERIPROC   glSamplerParameteri;
extern PFNGLSAMPLERPARAMETERFVPROC	glSamplerParameterfv;
extern PFNGLBINDSAMPLERPROC			glBindSampler;

extern PFNGLBINDTEXTUREPROC			glBindTexture;
extern PFNGLACTIVETEXTUREPROC		glActiveTexture;
extern PFNGLPIXELSTOREIPROC			glPixelStorei;
extern PFNGLGENTEXTURESPROC			glGenTextures;
extern PFNGLTEXPARAMETERIPROC		glTexParameteri;
extern PFNGLTEXIMAGE2DPROC			glTexImage2D;
extern PFNGLCOPYIMAGESUBDATAPROC	glCopyImageSubData;
extern PFNGLTEXSTORAGE2DPROC		glTexStorage2D;
extern PFNGLTEXSUBIMAGE2DPROC		glTexSubImage2D;

extern PFNGLGENERATEMIPMAPPROC		glGenerateMipmap;

extern PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC		glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTUREPROC		glFramebufferTexture;
extern PFNGLDRAWBUFFERSPROC				glDrawBuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC	glCheckFramebufferStatus;
extern PFNGLBLITFRAMEBUFFERPROC			glBlitFramebuffer;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameter;

extern PFNGLDEPTHFUNCPROC			glDepthFunc;
extern PFNGLDEPTHMASKPROC			glDepthMask;
extern PFNGLCLEARDEPTHFPROC			glClearDepthf;

extern PFNGLGETTEXIMAGEPROC			glGetTexImage;

extern PFNGLGETERRORPROC			glGetError;

extern PFNGLPOLYGONMODEPROC			glPolygonMode;

extern PFNGLCULLFACEPROC			glCullFace;

extern PFNGLFRONTFACEPROC			glFrontFace;

extern PFNGLGETBOOLEANVPROC			glGetBooleanv;

extern PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
extern PFNGLBLENDFUNCSEPARATEPROC	  glBlendFuncSeparate;

extern PFNGLBINDBUFFERBASEPROC		 glBindBufferBase;

extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC		glGetActiveUniformBlockiv;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC	glGetActiveUniformBlockName;
extern PFNGLGETACTIVEUNIFORMSIVPROC			glGetActiveUniformsiv;
extern PFNGLGETACTIVEUNIFORMNAMEPROC		glGetActiveUniformName;