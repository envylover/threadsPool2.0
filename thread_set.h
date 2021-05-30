#pragma once

//--------------------------------------------------------------------
//	thread_set.h
//	05/28/2021.				created.
//	05/30/2021.				lasted modified.
//--------------------------------------------------------------------

#ifndef _THREAD_SET_H
#define _THREAD_SET_H


                         /* header file*/
//--------------------------------------------------------------------

#include "handle_event.h"

#include <tuple>
#include <thread>
#include <mutex>
#include <functional>
#include <optional>
#include <assert.h>
#include <queue>
#include <optional>
#include <map>

//--------------------------------------------------------------------



namespace threadPool2 {


//--------------------------------------------------------------------

	enum class TaskStatus   //任务分类，确定是leader做还是followe做
	{
		LEADER,
		FOLLOWER
	};

//--------------------------------------------------------------------
// 
// 
// 	                   /* 同义词声明 */
//--------------------------------------------------------------------
	using FUN = std::optional<HANDLE>;

//--------------------------------------------------------------------


	             /*  线程集合类，负责线程对象的管理工作,内部有线程对象内部类 */
//--------------------------------------------------------------------
	class thread_set
	{
	public:
		enum class PoolStatus {//指示线程池状态，是否存在leader线程
			NO_LEADER,
			LEADER_EXIST
		};

	private:
		class Thread
		{


			                   /* 静态变量区 */
			//--------------------------------------------------------

			static thread_set*                   _pThread_set;//访问外部类

			static PoolStatus                    _status;//指示线程池状态

			static std::atomic_int               _threadCount;//记录线程对象个数

			static int                           _maxThreadCount;//线程对象数量上限

			static std::mutex                    _leader_mutex;//只允许一个leader存在

			static std::mutex                    _follower_mutex;//配合下方条件变量的使用

			static std::condition_variable       _cv;//用于唤醒follower线程

			//--------------------------------------------------------

			std::jthread                         _t{};//线程对象
			friend class thread_set;

		private:

			static void threadFun(void);//线程对象运行的函数

		public:
			Thread();
			Thread(Thread&&);
			Thread(const Thread&) = delete;
			Thread& operator = (const Thread&) = delete;
			Thread& operator = (Thread&& other) = delete;
			~Thread() {
			}
			auto get_id()->std::jthread::id; //获取线程id

		};

		struct FUNC//继承STL库的list容器，对读写增加同步锁
			:std::list<std::tuple<TaskStatus, FUN>>
		{
			FUNC() {}
			std::mutex  _mutex;
			auto pop_back()
				->std::optional<std::tuple<TaskStatus, FUN>>;

			void push_back(const std::tuple<TaskStatus, FUN>& _Val);

			void push_back(std::tuple<TaskStatus, FUN>&& _Val);
		}                                                 _function;


		std::map<decltype(Thread::_t.get_id()), Thread>   _thread_container;//以线程id作为key追踪线程对象
		
		struct  //继承STL里vector容器，对读写进行同步，用于记录已结束的线程对象
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



		std::optional<std::tuple<TaskStatus, FUN>> getTask();//线程对象获取任务
		bool wait();//等待事件产生
		void new_thread(size_t thread_count);//产生新的线程
		void remove_thread();//移除无用的线程对象
	public:
		thread_set(size_t threadCount = std::thread::hardware_concurrency() * 2);




		template<FK_EVENT _Ty>
		void addTask(handle_event<_Ty>&& task)//可根据事件类型判断对任务进行分类
		{
			if constexpr (isLeaderWork<handle_event<_Ty>>)
				_function.push_back({ TaskStatus::LEADER,std::forward<handle_event<_Ty>>(task) });
			else
				_function.push_back({ TaskStatus::FOLLOWER,std::forward<handle_event<_Ty>>(task) });

			_cv.notify_one();
		}


		void setMaxThreadsCount(size_t size);//设置线程池最大数量

		~thread_set();
	}; 


}


#endif