#include "lunarlady/math/Plane.hpp"

namespace lunarlady {
	namespace math {

		Plane::Plane() : mNormal(op::vec3::yAxisPositive), mDistance(0.0f) {
		}
		Plane::Plane(const vec3& pNormal, real pDistance) : mNormal(pNormal), mDistance(pDistance) {
		}
		Plane::Plane(const vec3& p0, const vec3& p1, const vec3& p2) : mNormal(op::vec3::yAxisPositive), mDistance(0.0f) {
			computeFrom3Points(p0, p1, p2);
		}
		Plane::Plane(const vec3& pNormal, const vec3& pPoint) : mNormal(op::vec3::yAxisPositive), mDistance(0.0f) {
			computeFromNormalAndPoint(pNormal, pPoint);
		}

		const vec3& Plane::getNormal() const {
			return mNormal;
		}
		real Plane::getDistance() const {
			return mDistance;
		}

		void Plane::setNormal(const vec3& pNormal) {
			mNormal = pNormal;
		}
		void Plane::setDistance(real pDistance) {
			mDistance = pDistance;
		}
		void Plane::computeFrom3Points(const vec3& p0, const vec3& p1, const vec3& p2) {
			// calculation and unexplanatory math names from plane tutorial at lighthouse3d.com
			vec3 v = p1; v -= p0;
			vec3 u = p2; u -= p0;
			mNormal = (v cross u).getNormalized();
			mDistance = - ( mNormal dot p0 );
		}
		void Plane::computeFromNormalAndPoint(const vec3& pNormal, const vec3& pPoint) {
			mNormal = pNormal;
			mDistance = - (mNormal dot pPoint);
		}
		real Plane::distanceBetween(const vec3& pPoint) const {
			const real a = mNormal dot pPoint + mDistance;
			return a;
		}
		PlaneRelation Plane::classify(const vec3& pPoint) const {
			const real distance = distanceBetween(pPoint);
			if( distance < 0.0f ) {
				return PR_BACK;
			}
			return PR_FRONT;
		}

		/** @todo rewrite? and test */
		void Plane::calculatePointOnPlane(const vec3& pPoint, real pSignedDistance, vec3* oPoint) const {
			// p = q - dist * n
			vec3& point = (*oPoint);
			point = pPoint;
			point-= mNormal * pSignedDistance;
		}
	}
}