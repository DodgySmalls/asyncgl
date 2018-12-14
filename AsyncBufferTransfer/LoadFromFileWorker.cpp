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
		Entity entity = self->mInputQueue->Dequeue();

		auto timer_start = std::chrono::steady_clock::now();

			entity.loadFromDisk();

			if (entity.wasLoaded())
			{
				self->mOutputQueue->Enqueue(entity);
			}
			else
			{
				fprintf(stderr, "ERROR : LoadFromFileWorker failed to load image from disk.\n");
			}

		auto timer_end = std::chrono::steady_clock::now();

		unsigned int duration = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start).count();
		self->mProfileNanoDurations.push_back(duration);
	}

	return NULL;
}

/**
	not threadsafe
*/
unsigned int LoadFromFileWorker::getMeanTaskTime()
{
	double running = 0.0;
	for (auto t : mProfileNanoDurations)
	{
		running += ((double)t) / ((double)mProfileNanoDurations.size());
	}

	return (unsigned int)running;
}