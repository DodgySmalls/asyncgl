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
#include <chrono>

//Include internals
#include "Vertex.h"
#include "Entity.h"
#include "FifoQueue.h"
#include "UploadWorker.h"
#define WORKER_NUM_UPLOADS	1
#include "LoadFromFileWorker.h"
#define WORKER_NUM_FILES	1


int waitConsole()
{
	int r = 0;
	fprintf(stderr, "Waiting... ");
	scanf_s("%d", &r);
	fprintf(stderr, " Continuing.\n");
	return r;
}

int main()
{
	if (!GlUtil::initializeGlfw())
	{
		exit(65);
	}
	GLFWwindow *window = GlUtil::initializeMainWindow();
	if (!window)
	{
		exit(66);
	}

	//glfwSwapInterval(0);				//allow opengl to flush the screen buffer without waiting for vsync

	glewExperimental = GL_TRUE;
	glewInit();

	if (!GlUtil::enableContextDebugging())
	{
		fprintf(stderr, "ERROR : Context was not flagged as debug.\n");
		if (waitConsole() == 6)
		{
			glfwDestroyWindow(window);
			glfwTerminate();
			exit(6);
		}
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);		// get renderer string
	const GLubyte* version = glGetString(GL_VERSION);		// version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	/*
			INITIALIZE OPENGL SHADERS
	*/
	char *vShaderStr = (char*)&(
		#include "vert.gl"
		)[0];

	char *fShaderStr = (char*)&(
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

	Entity ent = Entity("1.png");
	ent.loadFromDisk();
	ent.uploadToGpu();

	/*
		RUN
	*/
	
	//Entity list of fully loaded entities that may be used in rendering
	std::vector<Entity> entities;

	//Concurrent datastructures and workers for loading entities asynchronously
	FifoQueue<Entity> loadFromFileQueue = FifoQueue<Entity>();
	FifoQueue<Entity> uploadQueue = FifoQueue<Entity>();
	FifoQueue<Entity> readyQueue = FifoQueue<Entity>();
	LoadFromFileWorker *loaders[WORKER_NUM_FILES];
	for (int i = 0; i < WORKER_NUM_FILES; i++)
	{
		loaders[i] = new LoadFromFileWorker(&loadFromFileQueue, &uploadQueue);
		loaders[i]->run();
	}
	UploadWorker *uploaders[WORKER_NUM_UPLOADS];
	for (int i = 0; i < WORKER_NUM_FILES; i++)
	{
		uploaders[i] = new UploadWorker(window, &uploadQueue , &readyQueue);
		uploaders[i]->run();
	}


	unsigned int ticks = 0;
	bool launched = false;
	
	std::chrono::steady_clock::time_point last_snapshot_time = std::chrono::steady_clock::now();
	unsigned int snapshot_duration = 0;
	unsigned int snapshot_ticks = 0;

	loadFromFileQueue.Enqueue(Entity("1.png"));

	while (!glfwWindowShouldClose(window)) {
		auto timer_start = std::chrono::steady_clock::now();
		
		//Accept new entities from the ready queue until none are available
		//Explicitly define an entity with EMPTY_ENTITY_FILENAME to demonstrate here that this entity is not valid
		Entity freshEntity = Entity(EMPTY_ENTITY_FILENAME);
		for (;;)
		{
			if (readyQueue.isEmpty())
			{
				break;
			}
			
			freshEntity = readyQueue.Dequeue();
			if (freshEntity.textureId() != 0) {
				entities.push_back(freshEntity);
				printf("Got entity\n");
			}
			else
			{
				break;
			}
		}

		ticks++;
		unsigned int linearclock = ticks % 510;
		if (linearclock >= 255)
		{
			linearclock = 510 - linearclock;
		}
		float pulse = ((float)linearclock) / 255.0;


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		glUniform1f(unf_clock, pulse);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		if (entities.size() == 0)
		{
			//printf("entities empty, nothing to render\n");
		}

		for (Entity e : entities)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ent.textureId());
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLuint))); //reminder to draw offset vertices
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glUseProgram(0);
	
		glfwSwapBuffers(window);
		glfwPollEvents();

		auto timer_end = std::chrono::steady_clock::now();
		snapshot_ticks++;
		snapshot_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start).count();
		if (snapshot_duration >= 1000000000) {
			snapshot_duration -= 1000000000;
			printf("FPS: %u\n", snapshot_ticks);
			snapshot_ticks = 0;
		}
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

