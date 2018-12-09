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
	glClearColor(112, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(self->mContext);

	for (;;)
	{
		printf("UW   waiting for request...\n");
		Entity entity = self->mInputQueue->Dequeue();
		printf("UW   loading an entity!");
		entity.uploadToGpu();
		printf("UW   waiting for GPU sync");
		GLsync uploadComplete = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(uploadComplete, 0, 100);
		printf("UW   submitting ready request...");
		self->mOutputQueue->Enqueue(entity);
		printf("UW   task completed!");
	}

	return NULL;
}