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

	enum class TaskStatus   //������࣬ȷ����leader������followe��
	{
		LEADER,
		FOLLOWER
	};

//--------------------------------------------------------------------
// 
// 
// 	                   /* ͬ������� */
//--------------------------------------------------------------------
	using FUN = std::optional<HANDLE>;

//--------------------------------------------------------------------


	             /*  �̼߳����࣬�����̶߳���Ĺ�����,�ڲ����̶߳����ڲ��� */
//--------------------------------------------------------------------
	class thread_set
	{
	public:
		enum class PoolStatus {//ָʾ�̳߳�״̬���Ƿ����leader�߳�
			NO_LEADER,
			LEADER_EXIST
		};

	private:
		class Thread
		{


			                   /* ��̬������ */
			//--------------------------------------------------------

			static thread_set*                   _pThread_set;//�����ⲿ��

			static PoolStatus                    _status;//ָʾ�̳߳�״̬

			static std::atomic_int               _threadCount;//��¼�̶߳������

			static int                           _maxThreadCount;//�̶߳�����������

			static std::mutex                    _leader_mutex;//ֻ����һ��leader����

			static std::mutex                    _follower_mutex;//����·�����������ʹ��

			static std::condition_variable       _cv;//���ڻ���follower�߳�

			//--------------------------------------------------------

			std::jthread                         _t{};//�̶߳���
			friend class thread_set;

		private:

			static void threadFun(void);//�̶߳������еĺ���

		public:
			Thread();
			Thread(Thread&&);
			Thread(const Thread&) = delete;
			Thread& operator = (const Thread&) = delete;
			Thread& operator = (Thread&& other) = delete;
			~Thread() {
			}
			auto get_id()->std::jthread::id; //��ȡ�߳�id

		};

		struct FUNC//�̳�STL���list�������Զ�д����ͬ����
			:std::list<std::tuple<TaskStatus, FUN>>
		{
			FUNC() {}
			std::mutex  _mutex;
			auto pop_back()
				->std::optional<std::tuple<TaskStatus, FUN>>;

			void push_back(const std::tuple<TaskStatus, FUN>& _Val);

			void push_back(std::tuple<TaskStatus, FUN>&& _Val);
		}                                                 _function;


		std::map<decltype(Thread::_t.get_id()), Thread>   _thread_container;//���߳�id��Ϊkey׷���̶߳���
		
		struct  //�̳�STL��vector�������Զ�д����ͬ�������ڼ�¼�ѽ������̶߳���
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



		std::optional<std::tuple<TaskStatus, FUN>> getTask();//�̶߳����ȡ����
		bool wait();//�ȴ��¼�����
		void new_thread(size_t thread_count);//�����µ��߳�
		void remove_thread();//�Ƴ����õ��̶߳���
	public:
		thread_set(size_t threadCount = std::thread::hardware_concurrency() * 2);




		template<FK_EVENT _Ty>
		void addTask(handle_event<_Ty>&& task)//�ɸ����¼������ж϶�������з���
		{
			if constexpr (isLeaderWork<handle_event<_Ty>>)
				_function.push_back({ TaskStatus::LEADER,std::forward<handle_event<_Ty>>(task) });
			else
				_function.push_back({ TaskStatus::FOLLOWER,std::forward<handle_event<_Ty>>(task) });

			_cv.notify_one();
		}


		void setMaxThreadsCount(size_t size);//�����̳߳��������

		~thread_set();
	}; 


}


#endif