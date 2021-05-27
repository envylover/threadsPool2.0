#pragma once
#include <tuple>
#include <thread>
#include <coroutine>
#include <mutex>
#include <functional>
#include <optional>
#include <assert.h>
#include <queue>
#include <optional>
#include <map>

namespace threadPool2 {

	enum class TaskStatus
	{
		LEADER,
		FOLLOWER
	};
	using FUN = std::optional<std::function<void(void)>>;
	class thread_set
	{
	public:
		enum class PoolStatus {
			NO_LEADER,
			LEADER_EXIST
		};

	private:
		class Thread
		{
			static thread_set*                   _pThread_set;
			static PoolStatus                    _status;
			static std::atomic_int               _threadCount;
			static int                           _maxThreadCount;
			static std::mutex                    _leader_mutex;
			static std::mutex                    _follower_mutex;
			static std::condition_variable       _cv;


			std::jthread                         _t{ [this](void)->void {
				while (true)
				{
					if (_threadCount > _maxThreadCount)
					{
						--_threadCount;
						return ;
					}

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
			} };
			friend class thread_set;

		public:
			Thread() {
				++_threadCount;
				//_t.detach();
			}
			Thread(Thread&& other):_t(std::move(other._t)){
				if (&other == this)
					return;
			}
			Thread(const Thread&) = delete;
			Thread& operator = (const Thread&) = delete;
			Thread& operator = (Thread&& other) = delete;
			~Thread() {
			}
			auto get_id() {
				return _t.get_id();
			}
		};

		struct FUNC
			:std::list<std::tuple<TaskStatus, FUN>>
		{
			FUNC() {}
			std::mutex  _mutex;
			auto pop_back()
			->std::optional<std::tuple<TaskStatus, FUN>>
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
			void push_back(const std::tuple<TaskStatus, FUN>& _Val)
			{
				std::lock_guard lck(_mutex);
				std::list<std::tuple<TaskStatus, FUN>>::push_back(_Val);
			}
			void push_back(std::tuple<TaskStatus, FUN>&& _Val)
			{
				std::lock_guard lck(_mutex);
				std::list<std::tuple<TaskStatus, FUN>>::push_back(std::move(_Val));
			}
		}                                                 _function;
		std::map<decltype(Thread::_t.get_id()), Thread>   _thread_container;
		
		struct 
			:std::vector<decltype(Thread::_t.get_id())> 
		{
			std::mutex                                    _mutex;
			void push_back(decltype(Thread::_t.get_id()) id)
			{
				std::lock_guard lck(_mutex);
				std::vector<decltype(Thread::_t.get_id())>::push_back(id);
			}

			auto getContainer(void)
			{
				std::lock_guard lck(_mutex);
				std::vector<decltype(Thread::_t.get_id())>  temp;
				swap(temp);
				return temp;
			}
		}                                                 _INVALID_THREAD;
		
		std::mutex                                        _mutex;
		std::condition_variable                           _cv;
		


	private:
		std::optional<std::tuple<TaskStatus, FUN>> getTask();
		bool wait();
		void new_thread(size_t thread_count) {

			assert(thread_count >= 0);

			for (int i = 0; i < thread_count; ++i)
			{
				Thread t{};
				auto id = t.get_id();
				_thread_container.emplace(std::make_pair(id,std::move(t)));
			}

		}
		void remove_thread() {

			for (auto i : _INVALID_THREAD)
				_thread_container.erase(i);

		}
	public:
		thread_set(size_t threadCount):_function(){
			Thread::_maxThreadCount = threadCount;
			Thread::_pThread_set = this;
			new_thread(threadCount);
		}
		void addTask(TaskStatus status, FUN task);
		void setMaxThreadsCount(size_t size);

		~thread_set()
		{

			setMaxThreadsCount(0);
			Thread::_cv.notify_all();
			_cv.notify_one();
			_thread_container.clear();
		}
	}; 


}

