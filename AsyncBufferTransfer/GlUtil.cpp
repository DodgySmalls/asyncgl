#include "glUtil.h"

static void errorCallback(int error,
	const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

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

GLuint GlUtil::compileShader(char * shaderString, 
	GLenum shaderType)
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

GLuint GlUtil::compileProgram(GLuint vertexShaderId, 
	GLuint geometryShaderId, 
	GLuint fragmentShaderId)
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

bool GlUtil::initializeGlfw()
{
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	return true;
}

GLFWwindow * GlUtil::initializeMainWindow()
{
	GLFWwindow* window = glfwCreateWindow(800, 800, "Asynchronous Buffer Demo", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	return window;
}

/**
	GLFW does not afford us context management,
	the closest we can come is to create a new window as a child of the original, and not display it.
	This incurs some unfortunate overhead, but cannot be circumvented without using a different library.
	
	This function is not guaranteed to work under GLEW, as we don't necessarily know the location of our context.
	It will work in systems that uniformly return contexts associated with a single source.

	@pre This function may not be called unless the system has properly initialized GLFW
*/
GLFWwindow * GlUtil::initializeBackgroundWindow(GLFWwindow *parent)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

	return glfwCreateWindow(400, 400, "Background", NULL, parent);
}

bool GlUtil::enableContextDebugging()
{
	//examine context
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		fprintf(stdout, "Context is flagged as debug.\n");
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		if (glDebugMessageCallback)
		{
			glDebugMessageCallback(GlUtil::openGlDebugMessageCallback, nullptr);

			GLuint unusedIds = 0;
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
		}
		else
		{
			fprintf(stderr, "ERROR : glDebugMessageCallback is NULL, exiting.\n");
			int waitToProceed = 0;
			std::cin >> waitToProceed;
			exit(EXIT_FAILURE);
		}
		return true;
	}
	else
	{
		return false;
	}
}