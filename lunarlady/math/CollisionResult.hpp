#ifndef __COLLISIONRESULT_HPP
#define __COLLISIONRESULT_HPP

#include "lunarlady/math/vec3.hpp"
#include "lunarlady/math/Plane.hpp"
#include "lunarlady/math/Sphere.hpp"

namespace lunarlady {
	namespace math {
		class Ray;
		/** Holds the result after a collision.
		 * Impact referes to the impact point.
		 * Collision refers to the position of the
		 *   object where it collided along the move-direction,
		 *   theese are different (unless the object is a ray).
		 */
		class CollisionResult {
		public:
			CollisionResult();
			/** Gets the position of the object at collision point.
			 * If the object is a ray, this is the same as ImactPoint.
			 * @return the position of the object at collision point
			 */
			const math::vec3& getCollisionPoint() const;
			/** Gets the position of the impact.
			 * There may be other impacts, but this is one of them.
			 * If the object is a ray, this is the same as collision point.
			 * @return the position of the impact
			 */
			const math::vec3& getImpactPoint() const;
			const math::Plane& getImpactPlane() const;


			real getCollisionRatio() const;

			// ray - cylinder
			bool collision(const Ray& pRay, const Ray& line, const bool pInfiniteCylinder, real radius, real* oLineIntersection);

			// ray - sphere
			bool collision(const Ray& pRay, const Sphere& pSphere, const vec3& pSpherePosition);

			// ray - plane
			bool collision(const Ray& pRay, const Plane& p_plane);

			// swept point- static plane collision
			bool collision(	/* ray object */	const vec3& p_from, const vec3& p_to, const Plane& p_plane);
			// unverified
			// swept sphere - static plane collision
			bool collision(const Sphere& p_object, const vec3& p_from, const vec3& p_to, const Plane& p_plane);
		private:
			math::Plane m_impactPlane;
			math::vec3 m_collisionPoint; // collision pos of object
			math::vec3 m_impactPoint; // point on plane
			real m_collisionRatio;
		};
	};
};

#endif //__COLLISIONRESULT_HPP