#include <shared_thread.h>
#include <Windows.h>
#include <logger.h>
#include <mutex>

//Lock for the work queue
static std::mutex workerLock;
//Lock for operations that cannot be done from the working thread
static std::mutex workingLock;

shared_thread::shared_thread(int timeoutMillis)
{
	this->timeout = timeoutMillis;
	this->internalThread = std::thread([&]() {
		while (true) {
			if (!this->WorkCompleted()) {
				workingLock.lock();
				auto job = this->NextJob();
				if (job)
					job();
				else
					Logger::Debug("The next job was null, skipping...");
				workerLock.lock();
				this->workQueue.pop_front();
				workerLock.unlock();
				workingLock.unlock();
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
	if (!workingLock.try_lock()) {
		Logger::Debug("Work is being awaited from the same thread. This will cause a deadlock, so the work will not be awaited");
		return;
	}
	workingLock.unlock();
	while (!workQueue.empty()) {
		Sleep(this->timeout);
	}
}

void shared_thread::DoWork(std::function<void()> work)
{
	//Warning if you add work from worker thread!
	if (!workingLock.try_lock()) {
		Logger::Debug("Work was added from a shared thread to the same shared thread. DO NOT await for work to complete, or there will be a deadlock!!");
		Logger::Debug("Running work on the current thread to avoid deadlocks (this is likely expected anyways)");
		work();
		return;
	}
	workingLock.unlock();
	workerLock.lock();
	this->workQueue.push_back(work);
	workerLock.unlock();
}

std::function<void()> shared_thread::NextJob() {
	if (!this->WorkCompleted()) {
		return this->workQueue.front();
	}
	return 0;
}