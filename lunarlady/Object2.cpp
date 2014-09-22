#include "lunarlady/Object2.hpp"

namespace lunarlady {
	Object2::Object2() : mIsAlive(true), mIsEnabled(true) {
	}
	Object2::~Object2() {
	}

	bool Object2::isAlive() const {
		return mIsAlive;
	}

	bool Object2::isEnabled() const {
		return mIsEnabled;
	}
	void Object2::enable() {
		mIsEnabled = true;
	}
	void Object2::disable() {
		mIsEnabled = false;
	}

	void Object2::update(real iTime) {
		if( isEnabled() ) {
			doUpdate(iTime);
		}
	}
	void Object2::render(real iTime) {
		if( isEnabled() ) {
			doRender(iTime);
		}
	}
}