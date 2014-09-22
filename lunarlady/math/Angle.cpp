#include "lunarlady/math/Angle.hpp"

namespace lunarlady {
	namespace math {
		Angle::Angle() : mRadians(0) {
		}
		real Angle::inRadians() const {
			return mRadians;
		}
		real Angle::inDegrees() const {
			return radToDeg(mRadians);
		}

		Angle Angle::FromRadians(real iRadians) {
			return Angle(iRadians);
		}
		Angle Angle::FromDegrees(real iDegrees) {
			return Angle(degToRad(iDegrees));
		}
	}
}