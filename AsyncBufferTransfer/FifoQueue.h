#pragma once

//Include pthreads
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <semaphore.h>
typedef sem_t pthread_sem_t;

//Include standard libraries
#include <queue>



template<class T>
class FifoQueue
{
public:
	FifoQueue();
	~FifoQueue();
	void Enqueue(T object);	//<! Blocking
	T Dequeue();			//<! Blocking, blocks until resource available
	bool isEmpty();

private:
	std::queue<T> mQueue;
	pthread_mutex_t mAccess;
	pthread_sem_t mPost;
};

template <class T>
FifoQueue<T>::FifoQueue()
{
	mQueue = std::queue<T>();
	sem_init(&mPost, 0, 0);
	pthread_mutex_init(&mAccess, NULL);
}

template <class T >
FifoQueue<T>::~FifoQueue()
{
	sem_destroy(&mPost);
	pthread_mutex_destroy(&mAccess);
}

template <class T >
void FifoQueue<T>::Enqueue(T object)
{
	pthread_mutex_lock(&mAccess);
	
		mQueue.push(object);
	
	pthread_mutex_unlock(&mAccess);
	
	sem_post(&mPost);
}

template <class T>
T FifoQueue<T>::Dequeue()
{
	sem_wait(&mPost);
	
	T object;
	bool nonEmpty = true;
	
	pthread_mutex_lock(&mAccess);

		if (mQueue.size() > 0)
		{
			object = mQueue.front();
			mQueue.pop();
		}
		else
		{
			nonEmpty = false;
		}

	pthread_mutex_unlock(&mAccess);

	if (nonEmpty)
	{
		return object;
	}
	else
	{
		return T();
	}
}


/**
	This function does NOT imply threadsafety in calling Dequeue without blocking.
	It DOES allow us inference about the state of the queue.
*/
template <class T>
bool FifoQueue<T>::isEmpty()
{
	bool empty = false;

	pthread_mutex_lock(&mAccess);
		
		if (mQueue.size() == 0)
		{
			empty = true;
		}

	pthread_mutex_unlock(&mAccess);

	return empty;
}