#ifndef LL_MESSAGE_HPP
#define LL_MESSAGE_HPP

#include <string>

namespace lunarlady {
	enum ConsoleMessageType {
		CMT_ECHO,
		CMT_ERROR
	};
	void ConsoleMessage(ConsoleMessageType iType, const std::wstring& iMessage);
}

#endif