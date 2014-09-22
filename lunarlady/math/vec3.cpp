#include "lunarlady/math/vec3.hpp"
#include <cmath>
#include <cassert>

namespace lunarlady {
	namespace math {
		const bool vec3::operator==(const vec3& vec) const {
			return math::equal(getX(), vec.getX()) && math::equal(getY(), vec.getY()) && math::equal(getZ(), vec.getZ());
		}

		vec3::vec3(const vec3& iFrom, const vec3& iTo) {
			(*this) = iTo - iFrom;
		}

		real vec3::getLength() const {
			return squareRoot(getLengthSquared());
		}
		bool vec3::isUnit() const {
			return math::equal(1.0, getLengthSquared());
		}
		bool vec3::isZero() const {
			return math::equal(0.0, getLengthSquared());
		}
		real vec3::normalize() {
			real len = getLength();
			if( math::equal(len, 0.0f) ) len = 0.0001;
			(*this) /= len;
			return len;
		}

		namespace op {
			real vec3::lengthBetween(const ::lunarlady::math::vec3& from, const ::lunarlady::math::vec3& to) {
				return (from-to).getLength();
			}
			real vec3::lengthBetweenSquared(const ::lunarlady::math::vec3& from, const ::lunarlady::math::vec3& to) {
				return (from-to).getLengthSquared();
			}
			::lunarlady::math::vec3 vec3::getLinearInterpolation(const ::lunarlady::math::vec3& from, real value, const ::lunarlady::math::vec3& to) {
				return (to-from)*value + from;
			}
			void vec3::getLinearInterpolation(const ::lunarlady::math::vec3& from, real value, const ::lunarlady::math::vec3& to, ::lunarlady::math::vec3* out) {
				assert(out);
				*out = getLinearInterpolation(from, value, to);
			}

			real vec3::getCosAngleBetween(const ::lunarlady::math::vec3& a, const ::lunarlady::math::vec3& b) {
				assert(a.isUnit());
				assert(b.isUnit());
				return a dot b;
			}
			Angle vec3::getAngleBetween(const ::lunarlady::math::vec3& a, const ::lunarlady::math::vec3& b) {
				return Angle::FromRadians(acos(getCosAngleBetween(a, b)));
			}

			const ::lunarlady::math::vec3 vec3::origo(0,0,0);
			const ::lunarlady::math::vec3 vec3::xAxisPositive(1,0,0);
			const ::lunarlady::math::vec3 vec3::yAxisPositive(0,1,0);
			const ::lunarlady::math::vec3 vec3::zAxisPositive(0,0,1);
			const ::lunarlady::math::vec3 vec3::xAxisNegative(-1,0,0);
			const ::lunarlady::math::vec3 vec3::yAxisNegative(0,-1,0);
			const ::lunarlady::math::vec3 vec3::zAxisNegative(0, 0,-1);
		}
	}
}
