#ifndef LL_RGB_HPP
#define LL_RGB_HPP

#include "lunarlady/Types.hpp"

namespace lunarlady {
	class Rgb {
	public:
		explicit Rgb();
		Rgb(real iRed, real iGreen, real iBlue);

		void setRed(real iRed);
		void setGreen(real iGreen);
		void setBlue(real iBlue);

		real getRed() const;
		real getGreen() const;
		real getBlue() const;
	private:
		real mRed;
		real mGreen;
		real mBlue;
	};
}

#endif