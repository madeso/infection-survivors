#ifndef __RAY_HPP
#define __RAY_HPP

#include "lunarlady/math/vec3.hpp"

namespace lunarlady {
	namespace math {
		class Ray {
		public:
			Ray(const vec3& p_start, const vec3& p_direction);
			static Ray FromTo(const vec3& p_from, const vec3& p_to);

			void set(const vec3& p_from, const vec3& p_to);

			const vec3& getStart() const ;
			const vec3& getDirection() const ;
			vec3 getPoint(real p_location) const;
			void getPoint(real p_location, vec3& o_result) const;

			real getClosestPointOnLine(const vec3& pPoint) const;
		private:
			vec3 m_start;
			vec3 m_direction;
		};
	}
}

#endif // __RAY_HPP