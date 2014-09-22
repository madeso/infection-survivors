#include "lunarlady/Rgba.hpp"
#include "lunarlady/convertColorBetweenRed.hpp"
#include "lunarlady/convertColorBetweenGreen.hpp"
#include "lunarlady/convertColorBetweenBlue.hpp"

namespace lunarlady {
	Rgba::Rgba(const Color& iColor) : mRed(convert::ColorToRed(iColor)/255.0), mBlue(convert::ColorToGreen(iColor)/255.0), mGreen(convert::ColorToBlue(iColor)/255.0), mAlpha(1) {
	}
	Rgba::Rgba(real r, real g, real b, real a) : mRed(r), mGreen(g), mBlue(b), mAlpha(a) {
	}

	real Rgba::getRed() const {
		return mRed;
	}
	real Rgba::getGreen() const{
		return mGreen;
	}
	real Rgba::getBlue() const{
		return mBlue;
	}
	real Rgba::getAlpha() const{
		return mAlpha;
	}
}