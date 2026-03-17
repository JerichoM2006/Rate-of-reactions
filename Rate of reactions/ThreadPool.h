#pragma once

#include<vector>
#include<queue>
#include<functional>
#include<thread>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<future>
#include<iostream>

class ThreadPool
{
	using Task = std::function<void()>;
private:
	//Variables
	//Stopping flag
	std::atomic<bool> stopping{ false };

	//Lists/Queues
	std::vector<std::thread> threads;
	std::queue<Task> tasks;

	//Thread basics
	std::mutex mtx;
	std::condition_variable cv;

	//Barrier
	std::atomic<int> numBeforeBarrier{ 0 };
	std::condition_variable barrierCv;

	//Functions
	void threadFunctionality();

public:
	//Constructor/Destructor
	ThreadPool(int _threadAmount);
	~ThreadPool();

	//Modifiers
	template<class F>
	auto Enqueue(F&& f)->std::future<decltype(f())>
	{
		auto task = std::make_shared<std::packaged_task<decltype(f())()>>(f);
		{
			std::unique_lock<std::mutex> lock(this->mtx);
			this->tasks.push([task]() {(*task)(); });
			// Increment the barrier count
			this->numBeforeBarrier++;
			//std::cout << numBeforeBarrier << "\n";
		}
		// Notify a thread that a task is available
		this->cv.notify_one();

		//Return future
		return task->get_future();
	}
	void Barrier(); //Create a barrier to pool
};

