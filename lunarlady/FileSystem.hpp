#ifndef LL_FILESYSTEM_HPP
#define LL_FILESYSTEM_HPP

#include <string>

namespace lunarlady {
	void HandlePhysfsInitError(int iOk, const std::string& iDescription);
}

#endif