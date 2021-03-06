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
#define WORKER_NUM_FILES	2
#include "RequestSpawnWorker.h"

//Simulation/Demo Parameters
#define NUM_ROWS	8
#define NUM_COLUMNS	8
#define NUM_TASKS	64
#define PRELOAD_LEN 255

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
	glClearColor(0.25, 0, 0.25, 1);


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

	GLuint unf_clock = glGetUniformLocation(program, "clock");
	GLuint unf_translate = glGetUniformLocation(program, "translate");
	GLuint unf_scale = glGetUniformLocation(program, "scale");

	/*
			INITIAILIZE TRIANGLES
	*/
	Vertex vertices[] =
	{
		Vertex(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0,1)),
		Vertex(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0,0)),
		Vertex(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1,1)),
		Vertex(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1,0)),
	};

	GLuint indices[] =
	{
		0,1,2,
		3,2,1
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

	/*
		RUN
	*/

	//Entity list of fully loaded entities that may be used in rendering
	std::vector<Entity> entities;

	//Optionally preload a number of entities into the list before beginning the simulation
	for (int i = 0; i < PRELOAD_LEN; i++)
	{
		Entity entity = Entity("background.jpg");
		entity.loadFromDisk();
		entity.uploadToGpu();
		entities.push_back(entity);
	}

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

	//loadFromFileQueue.Enqueue(Entity("3.png"));
	loadFromFileQueue.Enqueue(Entity("background.jpg"));

	RequestSpawnWorker requestSpawner = RequestSpawnWorker(&loadFromFileQueue, NUM_TASKS);
	requestSpawner.run();

	unsigned int ticks = 0;
	bool launched = false;
	
	std::vector<unsigned int> profileFps;
	std::chrono::steady_clock::time_point last_snapshot_time = std::chrono::steady_clock::now();
	unsigned int snapshot_duration = 0;
	unsigned int snapshot_ticks = 0;

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
			}
			else
			{
				break;
			}
		}

		//We vary alpha smoothly as a crude visualization of FPS for the user
		//This technique will allow the eye to detect severe drops but is far more lenient than most real scenarios
		ticks++;
		unsigned int linearclock = ticks % 300;
		if (linearclock >= 150)
		{
			linearclock = 300 - linearclock;
		}
		float pulse = ((float)linearclock) / 150.0;

		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		glUniform1f(unf_clock, pulse);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		
		
		int slot = 0;
		float zoffset = 0.0f;
		glm::mat4x4 scale = GlUtil::getScaleMat( 2.0 / ((float) NUM_COLUMNS) , 2.0 / ((float)NUM_ROWS), 1.0);
		glUniformMatrix4fv(unf_scale, 1, GL_FALSE, &(scale[0][0]));
		for (Entity e : entities)
		{
			int row = (slot - (slot % NUM_COLUMNS)) / NUM_COLUMNS;
			int col = slot % NUM_COLUMNS;

			glm::mat4x4 translate = GlUtil::getTranslationMat((col * (2.0f / ((float)NUM_COLUMNS))) - 1.0f, (row * (2.0f / ((float)NUM_ROWS))) - 1.0f, zoffset);

			glm::vec4 test = glm::vec4(1.0, 1.0, 1.0, 1.0);
			test = translate * test;

			glUniformMatrix4fv(unf_translate, 1, GL_FALSE, &(translate[0][0]));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, e.textureId());
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

			slot++;
			if (slot >= NUM_COLUMNS * NUM_ROWS) {
				slot = 0;
				zoffset -= 0.05f;	//newer renders are infront of old ones
			}
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
			profileFps.push_back(snapshot_ticks);
			snapshot_ticks = 0;
		}

		if (entities.size() == NUM_TASKS + PRELOAD_LEN + 1) {
			double running = 0.0;
			printf("FPS:\n");
			for (auto fps : profileFps)
			{
				printf("%u,", fps);
				running += ((double)fps) / ((double)profileFps.size());
			}
			printf("\nMEAN FPS: %f\n", running);

			for (UploadWorker *w : uploaders)
			{
				printf("UW mut   : %u\n", w->getMeanUploadTime());
				printf("UW mst   : %u\n", w->getMeanSyncTime());
			}
			for (LoadFromFileWorker *w : loaders)
			{
				printf("LFFW mtt : %u\n", w->getMeanTaskTime());
			}
			break;
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

