#include "lunarlady/math/Sphere.hpp"
#include "lunarlady/math/vec3.hpp"
#include "lunarlady/math/Math.hpp"

namespace lunarlady {
	namespace math {
		Sphere::Sphere() : m_radius(0.0) {
		}
		Sphere::Sphere(const real p_radius) : m_radius(p_radius) {
		}
		void Sphere::includePoint(const vec3& p_point) {
			const real length = p_point.getLengthSquared();
			if( length > m_radius ) {
				m_radius = length;
			}
		}
		// since we have done a little speedup by not calling sqrt on each point in 
		void Sphere::doneBuilding() {
			m_radius = squareRoot(m_radius);
		}
		const real Sphere::getRadius() const {
			return m_radius;
		}
	}
}