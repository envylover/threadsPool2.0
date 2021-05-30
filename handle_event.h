#pragma once

//--------------------------------------------------------------------
//	handle_event.h
//	05/28/2021.				created.
//	05/30/2021.				lasted modified.
//--------------------------------------------------------------------

#ifndef _HANDLE_EVENT_H
#define _HANDLE_EVENT_H


						 /* header file*/
//--------------------------------------------------------------------
#include <optional>
#include <functional>
//--------------------------------------------------------------------


namespace threadPool2 {

	enum class FK_EVENT {
		FK_READ,
		FK_WRITE,
		FK_ACCEPT,
		FK_CLOSS
	};
	using HANDLE = std::function<void(void)>;
	template<FK_EVENT E>
	class handle_event 
		//�̳�std::function<void(void)>�࣬�����¼���������Ϣ
		:public HANDLE
	{
	public:
		handle_event(HANDLE handle) :HANDLE(handle) {}
	};


	template<typename _Ty>
	constexpr bool isLeaderWork //���¼����ָ࣬ʾ�Ƿ�Ӧ����leader����.
		= std::is_same_v<_Ty, handle_event<FK_EVENT::FK_ACCEPT>>
		|| std::is_same_v<_Ty, handle_event<FK_EVENT::FK_CLOSS>>;

}

#endif  