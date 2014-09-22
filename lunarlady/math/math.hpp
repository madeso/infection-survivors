#ifndef LLM_MATH_HPP
#define LLM_MATH_HPP

#include "lunarlady/Types.hpp"

namespace lunarlady {
	namespace math {
		const real radToDeg(const real rad);
		const real degToRad(const real deg);

		const real PI = 3.1415926535898f;
		const real TWO_PI = PI * 2.0f;
		//const real PI = 3.14159265358979323846f; // grabbed from a source - is it really needed
		const real ONE_DEG_IN_RADIAN = PI / 180;

		const bool logicXor(const bool a, const bool b);

		const real abs(const real a);
		const real Max(const real a, const real b);
		const real Min(const real a, const real b);

		const real floor(const real a);
		const real ceil(const real a);

		const bool withinRange(const real a, const real b, const real min);

		const bool equal(const real a, const real b);
		const bool equal0(const real a, const real b);
		const bool equal1(const real a, const real b);
		const bool equal2(const real a, const real b);
		const bool equal3(const real a, const real b);
		const bool equal4(const real a, const real b);
		const bool equal5(const real a, const real b);
		const bool equal6(const real a, const real b);
		const bool equal7(const real a, const real b);
		
		const real clampZero(const real a);
		const real limitRange(const real min, const real a, const real max);

		const real wrapRange(const real min, const real a, const real max);

		const real square(const real a);
		const real cube(const real a);
		const real squareRoot(const real a);

		const real sin(const real a);
		const real cos(const real a);
		const real tan(const real a);

		const real asin(const real a);
		const real acos(const real a);
		const real atan(const real a);

		const real interpolate(const real p_from, real p_fromOrTo, real p_to);

		real Map01ToMultiplier(real i01, real iBase);
	}
}

#endif