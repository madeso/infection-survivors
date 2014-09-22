#include "lunarlady/math/CollisionResult.hpp"
#include "lunarlady/math/Math.hpp"
#include "lunarlady/math/Ray.hpp"



namespace lunarlady {
	namespace math {
		CollisionResult::CollisionResult() : m_collisionRatio(0.0), m_collisionPoint(0,0,0), m_impactPoint(0,0,0) {
		}
		const math::vec3& CollisionResult::getCollisionPoint() const {
			return m_collisionPoint;
		}
		const math::vec3& CollisionResult::getImpactPoint() const {
			return m_impactPoint;
		}
		const math::Plane& CollisionResult::getImpactPlane() const {
			return m_impactPlane;
		}
		real CollisionResult::getCollisionRatio() const  {
			return m_collisionRatio;
		}

		bool CollisionResult::collision(const Ray& pRay, const Ray& line, const bool pInfiniteCylinder, real radius, real* oLineIntersection) {
			const vec3 normalizedDirection = pRay.getDirection().getNormalized();
			const vec3 source = pRay.getStart() - line.getStart();
			const vec3 normalizedCylinderAxis = line.getDirection().getNormalized();
			//const real cylinderLength = line.getDirection().getLength();
			vec3 crossNormal = normalizedDirection cross normalizedCylinderAxis;
			const real length = crossNormal.normalize();
			const real distanceFromCylinder = math::abs( source dot crossNormal );
			const real rayLength = pRay.getDirection().getLength();
			if( distanceFromCylinder <= radius ) {
				const vec3 u = source cross normalizedCylinderAxis;
				const real t = -(u dot crossNormal) / length;
				const vec3 v = (crossNormal cross normalizedCylinderAxis).getNormalized();
				const real s = math::abs(math::squareRoot(math::square(radius) - math::square(distanceFromCylinder)) / (normalizedDirection dot v));
				const real in = t-s;
				const real out = t+s;
				const real collisionRatio = math::Min(in, out) / rayLength;
				if( collisionRatio < 0.0f ) return false;
				const vec3 potentialCollisionPoint = pRay.getPoint(collisionRatio);
				if( !pInfiniteCylinder ) {
					const Plane startPlane(normalizedCylinderAxis, line.getStart() );
					if( startPlane.classify(potentialCollisionPoint) == PR_BACK ) {
						return false;
					}
					const Plane endPlane(normalizedCylinderAxis*-1, line.getStart()+line.getDirection() );
					if( endPlane.classify(potentialCollisionPoint) == PR_BACK ) {
						return false;
					}
				}

				if( oLineIntersection ){
					// what do do?
					// closes point on line
					*oLineIntersection = line.getClosestPointOnLine(potentialCollisionPoint);
				}

				m_collisionPoint = potentialCollisionPoint;
				m_collisionRatio = collisionRatio;

				return true;
			}
			else {
				return false;
			}
		}

		// done according to http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
		bool CollisionResult::collision(const Ray& pRay, const Sphere& pSphere, const vec3& pSpherePosition) {
			const real X0 = pRay.getStart().getX(); const real Y0 = pRay.getStart().getY(); const real Z0 = pRay.getStart().getZ();
			const real Xd = pRay.getDirection().getX(); const real Yd = pRay.getDirection().getY(); const real Zd = pRay.getDirection().getZ();
			const real Xc = pSpherePosition.getX(); const real Yc = pSpherePosition.getY(); const real Zc = pSpherePosition.getZ();
			const real Sr = pSphere.getRadius();

			const real a = square(Xd) + square(Yd) + square(Zd);
			const real b = 2 * (Xd * (X0 - Xc) + Yd * (Y0 - Yc) + Zd * (Z0 - Zc));
			const real c = square(X0 - Xc) + square(Y0 - Yc) + square(Z0 - Zc) - square(Sr);

			const real p = b / a;
			const real q = c / a;
			const real halfP = p/2;

			const real withinSqrt = square(halfP) - q;
			if( withinSqrt < 0.0 ) {
				// no collision
				return false;
			}
			else {
				const real halfDifference = squareRoot(withinSqrt);
				const real origin = -halfP;

				const real t0 = origin-halfDifference;
				const real t1 = origin-halfDifference;
				const real t = ( t0 > 0.0 )?
					(t0) : (t1);
				if( t < 0.0 ) {
					// collision occured behind
					return false;
				}
				else {
					m_collisionPoint = pRay.getPoint(t);
					m_collisionRatio = t;
					return true;
				}
			}
		}

		bool CollisionResult::collision(const Ray& pRay, const Plane& pPlane) {
			/** @todo give this a more meaningful name */
			const real numerator = -(( pPlane.getNormal() dot pRay.getStart()) + pPlane.getDistance());
			/** @todo give this a more meaningful name */
			const real denominator = pPlane.getNormal() dot pRay.getDirection();
			if( math::equal(denominator, 0.0) ) {
				// the normal and the ray are othogonal (perpendicular)
				// this means that the plane and the ray are parallel
				return false;
			}
			m_collisionRatio = numerator / denominator;
			pRay.getPoint(m_collisionRatio, m_collisionPoint);
			m_impactPoint = m_collisionPoint; // theese are the same for rays
			m_impactPlane = pPlane;
			return true;
		}

		// update fix and so on
		bool CollisionResult::collision(const vec3& p_from, const vec3& p_to, const Plane& p_plane) {
			const PlaneRelation fromRelation = p_plane.classify(p_from);
			const PlaneRelation toRelation = p_plane.classify(p_to);
			//if( fromRelation==PR_FRONT && fromRelation != toRelation ) {
			if( fromRelation != toRelation ) {
				vec3 direction = p_to; direction-=p_from;
				/** @todo give this a more meaningful name */
				const real numerator = -((p_plane.getNormal() dot p_from) + p_plane.getDistance());
				/** @todo give this a more meaningful name */
				const real denominator = p_plane.getNormal() dot direction;
				if( math::equal(denominator, 0.0) ) {
					// the normal and the ray are othogonal (perpendicular)
					// this means that the plane and the ray are parallel
					// and there is no unique point
					// should we really come here?
					return false;
				}
				m_collisionRatio = numerator / denominator;
				Ray ray(p_from, direction);
				ray.getPoint(m_collisionRatio, m_collisionPoint);
				m_impactPoint = m_collisionPoint; // theese are the same for rays
				m_impactPlane = p_plane;
				return true;
			}
			else{
				return false;
			}
		}

		// update fix and so on
		bool CollisionResult::collision(const Sphere& p_object, const vec3& p_from, const vec3& p_to, const Plane& p_plane) {
			const real distanceFrom = p_plane.distanceBetween(p_from);
			if( math::abs(distanceFrom) <= p_object.getRadius() ) {
				// touching the previous frame
				m_collisionPoint = p_from;
				m_impactPlane = p_plane;
				m_collisionRatio = 0.0f;

				//CONSOLE_MESSAGE(MT_MESSAGE, "Dist: " << distanceFrom );
				m_impactPlane.calculatePointOnPlane(p_from, -distanceFrom, &m_impactPoint);
				
				/*m_collisionPoint = p_from;
				math::vec3 normalScaled(m_impactPlane.getNormal()); normalScaled.scale(distanceFrom);
				m_collisionPoint += normalScaled;*/
				
				return true;
			}
			const real distanceTo = p_plane.distanceBetween(p_to);
			if( distanceFrom>p_object.getRadius() && distanceTo<p_object.getRadius() ) {
				/** @todo give this a more meaningful name */
				const real numerator = distanceFrom-p_object.getRadius();
				/** @todo give this a more meaningful name */
				const real denominator = distanceFrom-distanceTo;

				math::Ray ray = Ray::FromTo(p_from, p_to);
				
				m_collisionRatio = (numerator / denominator) - 0.001;
				ray.getPoint(m_collisionRatio, m_collisionPoint);

				m_impactPoint = m_collisionPoint;
				m_impactPoint = p_plane.getNormal() * (p_object.getRadius() + 0.001);
				//vec3 temp(p_plane.getNormal()); temp.scale(p_object.getRadius()+0.001); m_impactPoint-=temp;

				m_impactPlane = p_plane;
				return true;
			}
			return false;
		}
	};
};

//sphere / plane collision point
// http://www.gamedev.net/community/forums/topic.asp?topic_id=423907