#ifndef LL_TWEAK_HPP
#define LL_TWEAK_HPP

#include <string>
#include "lunarlady/Types.hpp"

namespace lunarlady {
	void TweakSingleImpl(const std::string& iName, bool* ioValue);
	void TweakSliderImpl(const std::string& iName, real* ioValue, real iMin, real iMax);

#define TweakSingle(iName, ioValue)				TweakSingleImpl(iName, ioValue)
#define TweakSlider(iName, ioValue, iMin, iMax)	TweakSliderImpl(iName, ioValue, iMin, iMax)
}

#endif