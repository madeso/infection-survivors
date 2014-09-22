#include "lunarlady/Loaded.hpp"
#include "sgl/sgl_Assert.hpp"

namespace lunarlady {
	Loaded::Loaded() : mUsage(0) {
	}
	Loaded::~Loaded() {
	}

	void Loaded::increaseUsage() {
		++mUsage;
	}

	void Loaded::decreaseUsage() {
		Assert(mUsage!=0, "This item isn't use anymore");
		--mUsage;
	}

	bool Loaded::isInUse() const {
		return mUsage != 0;
	}
}