#pragma once

//Include OpenGL libraries
#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#define GL_VERSION_MAJOR 4
#define GL_VERSION_MINOR 3
#include "glm/glm.hpp"

//Include standard libraries
#include <vector>
#include <stdlib.h>
#include <iostream>

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

	bool initializeGlfw();
	
	GLFWwindow * initializeMainWindow();
	
	GLFWwindow * initializeBackgroundWindow(GLFWwindow *parent);
	
	bool enableContextDebugging();

	glm::mat4x4 getScaleMat(float x, float y, float z);
	
	glm::mat4x4 getTranslationMat(float x, float y, float z);
}