
//--------------------------------------------------------------------
//	thread_set.cpp
//	05/28/2021.				created.
//	05/30/2021.				lasted modified.
//--------------------------------------------------------------------

//--------------------------------------------------------------------

#include "thread_set.h"

//--------------------------------------------------------------------


//--------------------------------------------------------------------
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
 //--------------------------------------------------------------------

 //--------------------------------------------------------------------
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
//--------------------------------------------------------------------

//--------------------------------------------------------------------
void threadPool2::thread_set::new_thread(size_t thread_count)
{

	assert(thread_count >= 0);

	for (int i = 0; i < thread_count; ++i)
	{
		Thread t{};
		auto id = t.get_id();
		_thread_container.emplace(std::make_pair(id, std::move(t)));
	}

}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
void threadPool2::thread_set::remove_thread()
{

	auto deadThread = _INVALID_THREAD.getContainer();

	for (auto i : deadThread)
		_thread_container.erase(i);

}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
threadPool2::thread_set::thread_set(size_t threadCount)
	:_function()
{
	Thread::_maxThreadCount = threadCount;
	Thread::_pThread_set = this;
	new_thread(threadCount);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
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
//--------------------------------------------------------------------


//--------------------------------------------------------------------
threadPool2::thread_set::~thread_set()
{
	setMaxThreadsCount(0);
	Thread::_cv.notify_all();
	_cv.notify_one();
	_thread_container.clear();
}
//--------------------------------------------------------------------

//--------------------------------------------------------------------

void threadPool2::thread_set::Thread::threadFun(void)
{
	while (true)
	{
		//--------------------------------------------------------------------
		if (_threadCount > _maxThreadCount)
		{//控制线程数量
			--_threadCount;
			_pThread_set->_INVALID_THREAD.push_back(std::this_thread::get_id());
			return;
		}
		//--------------------------------------------------------------------

		        /* 所有激活的线程争取成为leader，否则等待被唤醒 */
		//--------------------------------------------------------------------
		if (_leader_mutex.try_lock())
		{
			_status = PoolStatus::LEADER_EXIST;
		}
		else
		{
			std::unique_lock lck(_follower_mutex);
			_cv.wait(lck);
			continue;
		}
		//--------------------------------------------------------------------



		      /* leader线程等待任务分配，失败则放弃leader身份重新获取 */
		//--------------------------------------------------------------------
		if (_pThread_set->wait())
		{
			auto task = _pThread_set->getTask();
			if (!task)
			{
				_status = PoolStatus::NO_LEADER;
				_leader_mutex.unlock();
				continue;
			}
			auto [status, _task] = task.value();
			if (status == TaskStatus::FOLLOWER)
			{
				_status = PoolStatus::NO_LEADER;
				_leader_mutex.unlock();
				_cv.notify_one();
			}
			else
			{
				_leader_mutex.unlock();
			}
			if (_task)
				_task.value()();
		}
		else
			_leader_mutex.unlock();

	}
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
threadPool2::thread_set::Thread::Thread() :_t(&threadFun) {
	++_threadCount;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
threadPool2::thread_set::Thread::Thread(Thread&& other) 
	: _t(std::move(other._t)) {
	if (&other == this)
		return;
}
//--------------------------------------------------------------------



//--------------------------------------------------------------------
auto threadPool2::thread_set::Thread::get_id()->std::jthread::id
{
	return _t.get_id();
}
//--------------------------------------------------------------------



//--------------------------------------------------------------------
auto threadPool2::thread_set::FUNC::pop_back()
-> std::optional<std::tuple<TaskStatus, FUN>>
{
	std::lock_guard lck(_mutex);
	if (!empty())
	{
		auto temp = front();
		pop_front();
		return temp;
	}
	return std::nullopt;
}
//--------------------------------------------------------------------



//--------------------------------------------------------------------
void 
threadPool2::thread_set::FUNC::
push_back(const std::tuple<TaskStatus, FUN>& _Val)
{
	std::lock_guard lck(_mutex);
	std::list<std::tuple<TaskStatus, FUN>>::push_back(_Val);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
void 
threadPool2::thread_set::FUNC::
push_back(std::tuple<TaskStatus, FUN>&& _Val)
{
	std::lock_guard lck(_mutex);
	std::list<std::tuple<TaskStatus, FUN>>::push_back(std::move(_Val));
}
//--------------------------------------------------------------------

//--------------------------------------------------------------------

std::atomic_int threadPool2::thread_set::Thread::_threadCount = 0;

int threadPool2::thread_set::Thread::_maxThreadCount = 0;

threadPool2::thread_set::PoolStatus threadPool2::thread_set::Thread::_status = threadPool2::thread_set::PoolStatus::NO_LEADER;

threadPool2::thread_set* threadPool2::thread_set::Thread::_pThread_set = nullptr;

std::mutex threadPool2::thread_set::_singletonMx;

std::mutex threadPool2::thread_set::Thread:: _leader_mutex;
std::mutex threadPool2::thread_set::Thread:: _follower_mutex;
std::condition_variable threadPool2::thread_set::Thread:: _cv;


std::shared_ptr<threadPool2::thread_set> _hInstance{nullptr};

std::function<void(threadPool2::thread_set* p)>threadPool2::thread_set::deleter = [](thread_set* p)->void {
	if (p)
		delete p;
	p = nullptr;
};


//--------------------------------------------------------------------
void 
threadPool2::destoryThreadsPoolInstance()
{
	std::lock_guard<std::mutex> lck(thread_set::_singletonMx);
	if (_hInstance)
		_hInstance.reset();
}

//--------------------------------------------------------------------
std::weak_ptr<threadPool2::thread_set>
threadPool2::getThreadsPoolInstance(unsigned int threadsCount)
{
	if (_hInstance)
		return _hInstance;
	std::lock_guard<std::mutex> lck(thread_set::_singletonMx);
	if (_hInstance)
		return _hInstance;
	std::shared_ptr<thread_set> ptemp(new thread_set(threadsCount), thread_set::deleter);
	_hInstance.swap(ptemp);
	return _hInstance;
}
//--------------------------------------------------------------------
