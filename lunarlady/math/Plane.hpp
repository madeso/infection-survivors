#ifndef LLM_PLANE_HPP
#define LLM_PLANE_HPP

#include "lunarlady/math/vec3.hpp"

namespace lunarlady {
	namespace math {

		enum PlaneRelation {
			PR_FRONT,
			PR_BACK,
			PR_BOTH
		};

		class Plane {
		public:
			Plane();
			Plane(const vec3& p_normal, real m_distance);
			Plane(const vec3& p0, const vec3& p1, const vec3& p2);
			Plane(const vec3& p_normal, const vec3& p_point);

			const vec3& getNormal() const;
			real getDistance() const;

			void setNormal(const vec3& p_normal);
			void setDistance(real p_distance);

			void computeFrom3Points(const vec3& p0, const vec3& p1, const vec3& p2);
			void computeFromNormalAndPoint(const vec3& p_normal, const vec3& p_point);

			real distanceBetween(const vec3& p_point) const;

			PlaneRelation classify(const vec3& p_point) const;

			void calculatePointOnPlane(const vec3& p_point, real p_signedDistance, vec3* o_point) const;
		private:
			vec3 mNormal;
			real mDistance;
		};
	}
}

#endif