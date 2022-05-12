#include <shared_thread.h>
#include <Windows.h>
#include <logger.h>

shared_thread::shared_thread(int timeoutMillis)
{
	this->timeout = timeoutMillis;
	this->internalThread = std::thread([&]() {
		while (true) {
			if (!this->WorkCompleted()) {
				auto job = this->NextJob();
				job();
				this->workQueue.pop_front();
			}
			else {
				Sleep(this->timeout);
			}
		}
	});
	this->internalThread.detach();
}

bool shared_thread::WorkCompleted()
{
	return this->workQueue.empty();
}

void shared_thread::AwaitCompletion()
{
	//If we're on the same thread theres no reason to wait
	//Plus a side effect would be a deadlock
	if (this->internalThread.get_id() == std::this_thread::get_id()) {
		Logger::Print<Logger::WARNING>("Work is being awaited from the same thread. This will cause a deadlock, so the work will not be awaited");
		return;
	}
	while (!workQueue.empty()) {
		Sleep(this->timeout);
	}
}

void shared_thread::DoWork(std::function<void()> work)
{
	//Warning if you add work from worker thread!
	if (this->internalThread.get_id() == std::this_thread::get_id()) {
		Logger::Print<Logger::WARNING>("Work was added from a shared thread to the same shared thread. DO NOT await for work to complete, or there will be a deadlock!!");
	}
	this->workQueue.push_back(work);
}

std::function<void()> shared_thread::NextJob() {
	if (!this->WorkCompleted()) {
		return this->workQueue.front();
	}
}