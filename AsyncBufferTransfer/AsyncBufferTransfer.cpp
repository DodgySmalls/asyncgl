#pragma once

//Include pthreads
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

//Include OpenGL Extension Wrangler
#include <GL/glew.h>  

//Include OpenGL FrameWork
#include <GLFW/glfw3.h>  

//Include matrix library
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//Include STB image import library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "TextureWrapper.h"

//Include simple GL helper functions to smooth code
#include "GlUtil.h"

//Include the standard C++ headers  
#include <stdio.h>  
#include <stdlib.h>  
#include <iostream>
#include <string>
#include <memory>
#include <math.h>

//Include internals
#include "Vertex.h"


//Messy simplified task code

typedef struct {
	pthread_mutex_t *mu;
	bool *report;
	std::vector<TextureWrapper> *texs;
} taskArgs;

void *task(void * a)
{
	taskArgs *args = (taskArgs*)a;
	pthread_mutex_lock(args->mu);

	args->texs->push_back(TextureWrapper("2.jpg"));
	*(args->report) = true;


	pthread_mutex_unlock(args->mu);
	return NULL;
}


int main()
{
	/*
			INITIALIZE CONTEXT AND GLFW
	*/
	// start GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(800, 800, "Asynchronous Buffer Demo", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	/*
			INITIALIZE OPENGL SHADERS
	*/
	char* vShaderStr = (char*)&(
		#include "vert.gl"
		)[0];

	char * fShaderStr = (char*)&(
		#include "frag.gl"
		)[0];

	GLuint vertexShader = GlUtil::compileShader(vShaderStr, GL_VERTEX_SHADER);
	GLuint fragmentShader = GlUtil::compileShader(fShaderStr, GL_FRAGMENT_SHADER);
	GLuint program = GlUtil::compileProgram(vertexShader, 0, fragmentShader);

	//We use a clock value to allow demonstration of smooth render changes done by GPU calculation
	//It's important (in general) to avoid pushing these changes inside a VBO, though there are also more advanced
	//streaming methods for shader data that I'm not using here
	GLuint unf_clock = glGetUniformLocation(program, "clock");


	/*
			INITIAILIZE TRIANGLES
	*/
	Vertex vertices[] = 
	{
		Vertex(glm::vec3(-0.75f,  -0.25f,  0.0f), glm::vec2(0,1)),
		Vertex(glm::vec3(-0.75f,   0.25f,  0.0f), glm::vec2(0,0)),
		Vertex(glm::vec3(-0.25f,  -0.25f,  0.0f), glm::vec2(1,1)),
		Vertex(glm::vec3(-0.25f,   0.25f,  0.0f), glm::vec2(1,0)),

		Vertex(glm::vec3(0.25f,  -0.25f,  0.0f), glm::vec2(-1,-1)),
		Vertex(glm::vec3(0.25f,   0.25f,  0.0f), glm::vec2(-1,1)),
		Vertex(glm::vec3(0.75f,  -0.25f,  0.0f), glm::vec2(1,-1)),
		Vertex(glm::vec3(0.75f,   0.25f,  0.0f), glm::vec2(1,1))
	};
	GLuint indices[] =
	{
		0,1,2,
		3,2,1,
		4,5,6,
		7,6,5
	};

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLint positionAttributeId = glGetAttribLocation(program, "position");
	if (positionAttributeId < 0)
	{
		fprintf(stderr, "no attribute \"position\" found in entity program (%u)\n", program);
	}
	else
	{
		printf("EntityProgram (%u) attribute \"position\" = %d\n", program, positionAttributeId);
	}
	GLint texCoordsAttributeId = glGetAttribLocation(program, "textureCoords");
	if (texCoordsAttributeId < 0)
	{
		fprintf(stderr, "no attribute \"textureCoords\" found in entity program (%u)\n", program);
	} 
	else 
	{ 
		printf("EntityProgram (%u) attribute \"textureCoords\" = %d\n", program, texCoordsAttributeId);
	}
	glEnableVertexAttribArray(positionAttributeId);
	glEnableVertexAttribArray(texCoordsAttributeId);
	glVertexAttribPointer(texCoordsAttributeId, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, textureCoords)));
	glVertexAttribPointer(positionAttributeId, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	GLuint ebo = 0;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	//Textures that we loaded sequentially rather than asynchronously
	//This list exists as an artifact of how simple this implementation is
	std::vector<TextureWrapper> syncTextures;
	syncTextures.push_back(TextureWrapper("1.png"));
	syncTextures.push_back(TextureWrapper("2.jpg"));

	/*
		RUN
	*/
	pthread_t threads[1];
	int threada = 0;
	void ** ret = NULL;
	pthread_mutex_t simpleMutex;
	pthread_mutex_init(&simpleMutex, NULL);
	taskArgs args;
	args.mu = &simpleMutex;
	bool taskDone = false;
	args.report = &taskDone;
	args.texs = &syncTextures;

	unsigned int ticks = 0;
	bool launched = false;
	while (!glfwWindowShouldClose(window)) {
		ticks++;
		unsigned int linearclock = ticks % 510;
		if (linearclock >= 255)
		{
			linearclock = 510 - linearclock;
		}
		float pulse = ((float)linearclock) / 255.0;


		if (ticks > 255 && !launched)
		{
			//launch texture loader
			threada = pthread_create(&threads[0], NULL, task, &args);
			launched = true;
		}

		pthread_mutex_lock(&simpleMutex);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		glUniform1f(unf_clock, pulse);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, syncTextures.at(0).mTextureId);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

		if(taskDone)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, syncTextures.at(1).mTextureId);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLuint)));
		}
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		glUseProgram(0);
		pthread_mutex_unlock(&simpleMutex);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	/*
		CLEANUP
	*/
	glfwDestroyWindow(window);

	std::cout << "Enter anything to terminate ..." << std::endl;
	int x;
	std::cin >> x;

	glfwTerminate();



	return 0;
}

