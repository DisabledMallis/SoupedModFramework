#pragma once

#include <thread>
#include <deque>
#include <functional>

class shared_thread {
	std::thread internalThread;
	int timeout = 0;
	std::deque<std::function<void()>> workQueue;
public:
	shared_thread(int timeoutMillis);
	bool WorkCompleted();
	void AwaitCompletion();
	void DoWork(std::function<void()> work);
	std::function<void()> NextJob();
};