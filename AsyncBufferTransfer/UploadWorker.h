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

class UploadWorker
{
public:
	UploadWorker(GLFWwindow *parent, FifoQueue<Entity> *input, FifoQueue<Entity> *output);
	~UploadWorker();
	void run();

	std::vector<unsigned int> mProfileNanoDurations;
	std::vector<unsigned int> mProfileNanoSyncDurations;

	unsigned int getMeanUploadTime();
	unsigned int getMeanSyncTime();

private:
	GLFWwindow *mParent;
	GLFWwindow *mContext;
	bool isRunning;
	FifoQueue<Entity> *mInputQueue;
	FifoQueue<Entity> *mOutputQueue;

	static void *uploadTask(void *selfArgs);
};

struct UploadTaskArgs {
	UploadWorker *self;
	UploadTaskArgs(UploadWorker *s) : self(s) {}
};