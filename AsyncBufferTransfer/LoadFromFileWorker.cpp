#include "LoadFromFileWorker.h"

LoadFromFileWorker::LoadFromFileWorker(FifoQueue<Entity> *input, FifoQueue<Entity> *output)
{
	mInputQueue = input;
	mOutputQueue = output;
	isRunning = false;
}


LoadFromFileWorker::~LoadFromFileWorker()
{
}

/**
	Thread needs termination criteria, does not clean itself up
*/
void LoadFromFileWorker::run()
{
	//Only allow single instances of upload workers, not threadsafe
	//	i.e. LoadFromFileWorkers should only ever be visible to their parent 
	if (isRunning) {
		return;
	}

	pthread_t thread;
	int tid = 0;

	printf("this: %p", this);
	LoadFileTaskArgs *args = (LoadFileTaskArgs *)malloc(sizeof(LoadFileTaskArgs));
	*args = LoadFileTaskArgs(this);

	tid = pthread_create(&thread, NULL, loadFileTask, args);

	isRunning = true;
}

void *LoadFromFileWorker::loadFileTask(void * selfArgs)
{
	LoadFromFileWorker *self = ((LoadFileTaskArgs *)selfArgs)->self;

	for (;;)
	{
		printf("LFFW waiting for request...\n");
		Entity entity = self->mInputQueue->Dequeue();
		printf("LFFW loading an entity!");
		entity.loadFromDisk();
		printf("LFFW submitting upload request...");
		self->mOutputQueue->Enqueue(entity);
		printf("LFFW task completed!");
	}

	return NULL;
}