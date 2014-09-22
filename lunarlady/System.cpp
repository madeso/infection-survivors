#include "lunarlady/System.hpp"
#include "lunarlady/Log.hpp"

namespace lunarlady {
	System::System(const std::string& iName) : mName(iName) {
		LOG1( "Initializing " << mName << " system");
	}
	System::~System() {
		LOG1( "De-Initializing " << mName << " system");
	}

	const std::string& System::getName() const {
		return mName;
	}
}