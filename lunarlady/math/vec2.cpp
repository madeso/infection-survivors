#include "lunarlady/math/vec2.hpp"
#include "lunarlady/math/math.hpp"
#include <cmath>

namespace lunarlady {
	namespace math {

		const bool vec2::operator==(const vec2& vec) const {
			return math::equal(getX(), vec.getX()) && math::equal(getY(), vec.getY());
		}

		real vec2::getLength() const {
			return squareRoot(getLengthSquared());
		}

		namespace op {
			namespace vec2 { 
				real lengthBetween(const ::lunarlady::math::vec2& from, const ::lunarlady::math::vec2& to) {
					return (from-to).getLength();
				}
				real lengthBetweenSquared(const ::lunarlady::math::vec2& from, const ::lunarlady::math::vec2& to) {
					return (from-to).getLengthSquared();
				}
				const ::lunarlady::math::vec2 Truncate(const ::lunarlady::math::vec2& iVector, real iMax) {
					const real length = iVector.getLength();
					if( length > iMax ) {
						return iVector * (iMax / length);
					}
					else {
						return iVector;
					}
				}
			}
		}

	}
}