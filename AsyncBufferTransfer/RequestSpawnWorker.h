#pragma once
//Include pthreads
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

//Include OpenGL libraries
#include <GL/glew.h>  
#include "GLFW/glfw3.h"

//Include standard libraries
#include <chrono>
#include <thread>
#include <random>

//Include internals
#include "Entity.h"
#include "FifoQueue.h"
#include "GlUtil.h"

class RequestSpawnWorker
{
public:
	RequestSpawnWorker(FifoQueue<Entity> *output, unsigned int numGenerations);
	~RequestSpawnWorker();
	void run();
private:
	GLFWwindow *mParent;
	bool isRunning;
	FifoQueue<Entity> *mOutputQueue;
	unsigned int mNumGenerations;

	static void *spawnRequestTask(void *selfArgs);
};

struct SpawnRequestTaskArgs {
	RequestSpawnWorker *self;

	SpawnRequestTaskArgs(RequestSpawnWorker *s) : self(s) {}
};