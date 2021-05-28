#include "thread_set.h"

 std::optional<std::tuple<threadPool2::TaskStatus, threadPool2::FUN>>
	 threadPool2::thread_set::getTask()
{
	if (!_function.empty())
	{
		auto temp = _function.pop_back();
		return { temp };
	}
	return std::nullopt;
}

bool threadPool2::thread_set::wait()
{
	if (!_function.empty())
		return true;
	{
		std::unique_lock lck(_mutex);
		_cv.wait(lck);
	}

	return true;
}

void threadPool2::thread_set::addTask(TaskStatus status, FUN task)
{
	_function.push_back({ status,task });
	_cv.notify_one();
}

void threadPool2::thread_set::setMaxThreadsCount(size_t maxSize)
{
	assert(maxSize >= 0);


	Thread::_maxThreadCount = maxSize;

	if (Thread::_threadCount < maxSize)
	{
		
		new_thread(maxSize - Thread::_threadCount);
	}
	else
	{
		remove_thread();
	}

	Thread::_cv.notify_all();


}


std::atomic_int threadPool2::thread_set::Thread::_threadCount = 0;

int threadPool2::thread_set::Thread::_maxThreadCount = 0;

threadPool2::thread_set::PoolStatus threadPool2::thread_set::Thread::_status = threadPool2::thread_set::PoolStatus::NO_LEADER;

threadPool2::thread_set* threadPool2::thread_set::Thread::_pThread_set = nullptr;


std::mutex threadPool2::thread_set::Thread:: _leader_mutex;
std::mutex threadPool2::thread_set::Thread:: _follower_mutex;
std::condition_variable threadPool2::thread_set::Thread:: _cv;