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
		return;
	}
	while (!workQueue.empty()) {
		Sleep(this->timeout);
	}
}

void shared_thread::DoWork(std::function<void()> work)
{
	this->workQueue.push_back(work);
}

std::function<void()> shared_thread::NextJob() {
	if (!this->WorkCompleted()) {
		return this->workQueue.front();
	}
}