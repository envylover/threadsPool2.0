#pragma once
#include <optional>
#include <functional>

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
		:public HANDLE
	{
	public:
		handle_event(HANDLE handle) :HANDLE(handle) {}
	};


	template<typename _Ty>
	constexpr bool isLeaderWork = std::is_same_v<_Ty, handle_event<FK_EVENT::FK_ACCEPT>>
		|| std::is_same_v<_Ty, handle_event<FK_EVENT::FK_CLOSS>>;

}