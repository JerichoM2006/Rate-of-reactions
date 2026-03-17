#include "ThreadPool.h"

//Constructor/Destructor
ThreadPool::ThreadPool(int _threadAmount)
{
	//Create the threads
	for (int i = 0; i < _threadAmount; i++)
	{
		this->threads.push_back(std::thread(&ThreadPool::threadFunctionality, this));
	}
}
ThreadPool::~ThreadPool()
{
	//Turn stopping variable true
	{
		std::unique_lock<std::mutex> lock{ this->mtx };
		this->stopping = true;
	}

	//Notify this to all threads
	this->cv.notify_all();

	//Join all threads
	for (auto& thread : threads)
	{
		thread.join();
	}
}

//Thread functionality
void ThreadPool::threadFunctionality()
{
	while (true)
	{
		Task task;

		{
			//Wait until there is available task or stopped
			std::unique_lock<std::mutex> lock{ this->mtx };
			this->cv.wait(lock, [=] {return this->stopping || !this->tasks.empty(); });
			
			//Exit if stopped
			if (this->stopping && this->tasks.empty()) break;

			//Implement task from queue
			task = std::move(this->tasks.front());
			this->tasks.pop();
		}

		//Run task
		task();

		//Check if there's still tasks before barrier
		if (this->numBeforeBarrier > 0)
		{
			//Decrement tasks before barrier
			std::unique_lock<std::mutex> lock{ this->mtx };
			this->numBeforeBarrier--;
			//If tasks finished notify that barrier can be removed
			if (this->numBeforeBarrier <= 0) this->barrierCv.notify_all();
		}
	}
}

//Modifiers
void ThreadPool::Barrier()
{
	std::unique_lock<std::mutex> lock{ this->mtx };
	//Wait until all tasks before the barrier have completed
	barrierCv.wait(lock, [=] {return this->numBeforeBarrier <= 0; });
	//Reset the barrier counter for future barriers
	this->numBeforeBarrier = 0;
}