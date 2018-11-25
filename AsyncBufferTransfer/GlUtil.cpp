#include "glUtil.h"

void APIENTRY GlUtil::openGlDebugMessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{

	fprintf(stderr, "---------------------opengl-callback-start------------\n");
	fprintf(stderr, "message: %s\n", message);
	fprintf(stderr, "type: ");
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		fprintf(stderr, "ERROR");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		fprintf(stderr, "DEPRECATED_BEHAVIOR");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		fprintf(stderr, "UNDEFINED_BEHAVIOR");
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		fprintf(stderr, "PORTABILITY");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		fprintf(stderr, "PERFORMANCE");
		break;
	case GL_DEBUG_TYPE_OTHER:
		fprintf(stderr, "OTHER");
		break;
	}
	fprintf(stderr, "\n");

	fprintf(stderr, "id: %u\n", id);
	fprintf(stderr, "severity: ");
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		fprintf(stderr, "LOW");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		fprintf(stderr, "MEDIUM");
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		fprintf(stderr, "HIGH");
		break;
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "---------------------opengl-callback-end--------------\n");
}

GLuint GlUtil::compileShader(char * shaderString, GLenum shaderType)
{
	if (shaderType != GL_VERTEX_SHADER && 
		shaderType != GL_GEOMETRY_SHADER &&
		shaderType != GL_FRAGMENT_SHADER)
	{
		fprintf(stderr, "ERR >> GlUtil::compileShader()\n       shaderType (%d) was not one of accepted types.", shaderType);
		return 0;
	}

	//Attempt to compile shader
	GLuint shaderId = glCreateShader(shaderType);
	const char * constShaderString = shaderString;
	glShaderSource(shaderId, 1, &constShaderString, NULL);
	glCompileShader(shaderId);

	//Check shader compilation outcome
	GLint success = GL_FALSE;
	int infoLogLength;
	std::vector<char> errMsg;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (!success) {
		errMsg.resize(infoLogLength);
		glGetShaderInfoLog(shaderId, infoLogLength, NULL, &errMsg[0]);
		fprintf(stderr, "ERR >> GlUtil::compileShader()\n       ");
		switch (shaderType)
		{
			case GL_VERTEX_SHADER: fprintf(stderr, "Vertex");
			case GL_GEOMETRY_SHADER: fprintf(stderr, "Geometry");
			case GL_FRAGMENT_SHADER: fprintf(stderr, "Fragment");
			default: fprintf(stderr, "<UnknownShaderType>");
		}
		fprintf(stderr, " Shader Compilation Error:\n%s\n", &errMsg[0]);
		return 0;
	}
	return shaderId;
}

GLuint GlUtil::compileProgram(GLuint vertexShaderId, GLuint geometryShaderId, GLuint fragmentShaderId)
{
	// Link the program
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	if (geometryShaderId != 0)
	{
		glAttachShader(programId, geometryShaderId);
	}
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);

	/// Check the program
	GLint success = GL_FALSE;
	int infoLogLength;
	std::vector<char> errMsg;
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (!success) {
		errMsg.resize(infoLogLength);
		glGetProgramInfoLog(programId, infoLogLength, NULL, &errMsg[0]);
		fprintf(stderr, "Program Link Failed:\n%s\n", &errMsg[0]);
		fprintf(stderr, "Note that vertexShader(%u), geometryShader(%u), and fragmentShader(%u) have not been deleted.\n", vertexShaderId, geometryShaderId, fragmentShaderId);
		return 0;
	}

	glDeleteShader(vertexShaderId);
	if (geometryShaderId != 0)
	{
		glDeleteShader(geometryShaderId);
	}
	glDeleteShader(fragmentShaderId);

	return programId;
}