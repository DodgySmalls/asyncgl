#pragma once
//Include pthreads
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

//Include OpenGL libraries
#include <GL/glew.h>  
#include "GLFW/glfw3.h"

//Include standard libraries
#include <chrono>

//Include internals
#include "Entity.h"
#include "FifoQueue.h"
#include "GlUtil.h"

class LoadFromFileWorker
{
public:
	LoadFromFileWorker(FifoQueue<Entity> *input, FifoQueue<Entity> *output);
	~LoadFromFileWorker();
	void run();
	unsigned int getMeanTaskTime();

private:
	GLFWwindow *mParent;
	bool isRunning;
	FifoQueue<Entity> *mInputQueue;
	FifoQueue<Entity> *mOutputQueue;
	
	std::vector<unsigned int> mProfileNanoDurations;

	static void *loadFileTask(void *selfArgs);
};

struct LoadFileTaskArgs {
	LoadFromFileWorker *self;

	LoadFileTaskArgs(LoadFromFileWorker *s) : self(s) {}
};