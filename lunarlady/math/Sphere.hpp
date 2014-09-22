#ifndef __SPHERE_HPP
#define __SPHERE_HPP

#include "lunarlady/Types.hpp"

namespace lunarlady {
	namespace math {
		class vec3;

		class Sphere {
		public:
			Sphere();
			Sphere(const real p_radius);
			void includePoint(const vec3& p_point);
			void doneBuilding();
			const real getRadius() const;
		private:
			real m_radius;
		};
	}
}

#endif