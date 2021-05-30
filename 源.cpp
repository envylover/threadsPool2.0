
#include "thread_set.h"

#include <iostream>

int i = 0;
std::mutex mt;

void fun(int j)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	std::lock_guard<std::mutex> lck(mt);
	std::cout << "thread id\t: " << std::this_thread::get_id() << "\t: " << i++ << std::endl;
}

int main()
{
	{
		threadPool2::thread_set tm(3000);
		for (int j = 0; j < 100000; j++)
			tm.addTask(threadPool2::handle_event<threadPool2::FK_EVENT::FK_READ>(std::bind(fun, j)));
		////std::this_thread::sleep_for(std::chrono::milliseconds(520));
		////tm.setMaxThreadsCount(6);
		////std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		////tm.setMaxThreadsCount(15);
		std::this_thread::sleep_for(std::chrono::milliseconds(100000));
		//std::this_thread::sleep_for(std::chrono::milliseconds(100000));
		int a = 0;
	}


	/*threadPool2::thread_set tm(3000);

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	tm.setMaxThreadsCount(300);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	tm.setMaxThreadsCount(30);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	tm.setMaxThreadsCount(10);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	tm.setMaxThreadsCount(5);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	tm.setMaxThreadsCount(15);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));*/

	return 0;
}


