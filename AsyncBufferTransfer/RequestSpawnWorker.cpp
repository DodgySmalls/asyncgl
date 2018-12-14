#include "RequestSpawnWorker.h"

RequestSpawnWorker::RequestSpawnWorker(FifoQueue<Entity> *output, unsigned int numGenerations)
{
	mOutputQueue = output;
	mNumGenerations = numGenerations;
	isRunning = false;

}


RequestSpawnWorker::~RequestSpawnWorker()
{
}

/**
	Thread needs termination criteria, does not clean itself up
*/
void RequestSpawnWorker::run()
{
	//Only allow single instances of workers, not threadsafe
	//	i.e. RequestSpawnWorkers should only ever be visible to their parent 
	if (isRunning) {
		return;
	}

	pthread_t thread;
	int tid = 0;

	printf("this: %p", this);
	SpawnRequestTaskArgs *args = (SpawnRequestTaskArgs *)malloc(sizeof(SpawnRequestTaskArgs));
	*args = SpawnRequestTaskArgs(this);

	tid = pthread_create(&thread, NULL, spawnRequestTask, args);

	isRunning = true;
}

void *RequestSpawnWorker::spawnRequestTask(void * selfArgs)
{
	RequestSpawnWorker *self = ((SpawnRequestTaskArgs *)selfArgs)->self;
	
	//Allow for warmup period for rendering to stabilize
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	
	std::default_random_engine generator;
	std::exponential_distribution<double> expDistribution(1.0f /  300.0f);

	for (int i = 0; i < self->mNumGenerations; i++)
	{
		double arrivalTime = expDistribution(generator);

		std::this_thread::sleep_for(std::chrono::milliseconds((int)arrivalTime));

		Entity entity = Entity("foreground.png");

		self->mOutputQueue->Enqueue(entity);
	}

	return NULL;
}