#ifndef LL_RGBA_HPP
#define LL_RGBA_HPP

#include "lunarlady/Types.hpp"
#include "lunarlady/Color.hpp"

namespace lunarlady {

	class Rgba {
	public:
		Rgba(const Color& iColor);
		Rgba(real r, real g, real b, real a);

		real getRed() const;
		real getGreen() const;
		real getBlue() const;
		real getAlpha() const;
	private:
		real mRed;
		real mGreen;
		real mBlue;
		real mAlpha;
	};

}

#endif