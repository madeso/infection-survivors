#include "lunarlady/Rgb.hpp"

namespace lunarlady {
	Rgb::Rgb() : mRed(1), mGreen(1), mBlue(1) {
	}
	Rgb::Rgb(real iRed, real iGreen, real iBlue) : mRed(iRed), mGreen(iGreen), mBlue(iBlue) {
	}

	void Rgb::setRed(real iRed) {
		mRed = iRed;
	}
	void Rgb::setGreen(real iGreen) {
		mGreen = iGreen;
	}
	void Rgb::setBlue(real iBlue) {
		mBlue = iBlue;
	}

	real Rgb::getRed() const {
		return mRed;
	}
	real Rgb::getGreen() const {
		return mGreen;
	}
	real Rgb::getBlue() const {
		return mBlue;
	}
}