#include <iostream>

#include "lunarlady/Log.hpp"

namespace lunarlady {
	void Log(const std::string& iMessage, const std::string& iFunction, const std::string& iFile, const int iLine) {
		::std::clog << iFile.c_str() << "(" << iLine << ") : " << iMessage.c_str() << std::endl;
	}
}