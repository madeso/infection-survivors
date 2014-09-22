#ifndef LLM_ANGLE_HPP
#define LLM_ANGLE_HPP

#include "lunarlady/math/math.hpp"

namespace lunarlady {
	namespace math {
		class Angle {
		public:
			Angle();
			real inRadians() const ;
			real inDegrees() const ;
			static Angle FromRadians(real iRadians);
			static Angle FromDegrees(real iDegrees);
		private:
			explicit Angle(real radians) : mRadians(radians) {}
			real mRadians;
		};
	}
}

#endif