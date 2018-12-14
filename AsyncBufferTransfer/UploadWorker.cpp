#include "UploadWorker.h"

UploadWorker::UploadWorker(GLFWwindow *parent, FifoQueue<Entity> *input, FifoQueue<Entity> *output)
{
	mParent = parent;
	mInputQueue = input;
	mOutputQueue = output;
	isRunning = false;
}


UploadWorker::~UploadWorker()
{
}

/**
	Thread needs termination criteria, does not clean itself up
*/
void UploadWorker::run()
{
	//Only allow single instances of upload workers, not threadsafe
	//	i.e. UploadWorkers should only ever be visible to their parent 
	if (isRunning) {
		return;
	}

	
	pthread_t thread;
	int tid = 0;
	
	mContext = GlUtil::initializeBackgroundWindow(mParent);



	if (mContext)
	{
		UploadTaskArgs *args = (UploadTaskArgs *)malloc(sizeof(UploadTaskArgs));
		*args = UploadTaskArgs(this);
		
		tid = pthread_create(&thread, NULL, uploadTask, args);

		isRunning = true;
	}
	else
	{
		fprintf(stderr, "ERROR : Failed to initialize uploader context");
	}
}

void *UploadWorker::uploadTask(void * selfArgs)
{
	UploadWorker *self = ((UploadTaskArgs *)selfArgs)->self;
	glfwMakeContextCurrent(self->mContext);
	
	//Visual test to ensure context functioning properly
	//glClearColor(112, 0, 0, 0);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glfwSwapBuffers(self->mContext);

	for (;;)
	{
		Entity entity = self->mInputQueue->Dequeue();

		auto timer_start = std::chrono::steady_clock::now();

			entity.uploadToGpu();

		auto timer_end = std::chrono::steady_clock::now();
		unsigned int duration = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start).count();
		self->mProfileNanoDurations.push_back(duration);

		timer_start = std::chrono::steady_clock::now();

			GLsync uploadComplete = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			glClientWaitSync(uploadComplete, 0, 100);

		timer_end = std::chrono::steady_clock::now();
		duration = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start).count();
		self->mProfileNanoSyncDurations.push_back(duration);


		self->mOutputQueue->Enqueue(entity);
	}

	return NULL;
}

/**
	not threadsafe
*/
unsigned int UploadWorker::getMeanUploadTime()
{
	double running = 0.0;
	for (auto t : mProfileNanoDurations)
	{
		running += ((double)t) / ((double)mProfileNanoDurations.size());
	}

	return (unsigned int)running;
}

/**
	not threadsafe
*/
unsigned int UploadWorker::getMeanSyncTime()
{
	double running = 0.0;
	for (auto t : mProfileNanoSyncDurations)
	{
		running += ((double)t) / ((double)mProfileNanoDurations.size());
	}

	return (unsigned int)running;
}