#pragma once

#include <GL/glew.h>  
#include <GLFW/glfw3.h>

#include <vector>
#include <stdlib.h>

namespace GlUtil
{
	void APIENTRY openGlDebugMessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam);

	GLuint compileShader(char * shaderString, 
		GLenum shaderType);

	GLuint compileProgram(GLuint vertexShaderId, 
		GLuint geometryShaderId,
		GLuint fragmentShaderId);
}