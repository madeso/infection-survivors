#ifndef LLM_QUATERNION_HPP
#define LLM_QUATERNION_HPP

#include "lunarlady/math/vec3.hpp"

namespace lunarlady {
	namespace math {
		class Angle;
		class Quaternion {
		public:
			Quaternion(const Quaternion& p_other);
			Quaternion(const real p_x, const real p_y, const real p_z, const real p_w);
			Quaternion(const vec3& axis, const real theta);
			Quaternion(const vec3& vec);
			Quaternion();
			~Quaternion();
			void setEuler(const real p_x, const real p_y, const real p_z);
			void setRotation(const vec3& axis, const real theta);
			real length() const;
			real lengthSquared() const;
			bool isUnit() const;
			// returns length()
			real normalize();
			//void toMatrix(Matrix& p_out);
			void conjugate();
			Quaternion getConjugate() const;
			const Quaternion& operator*=(const Quaternion& p_quat);
			Quaternion operator*(const Quaternion& a) const;
			void set(const real p_x, const real p_y, const real p_z, const real p_w);
			void set(const Quaternion& p_quat);
			void setIdentity();
					
			vec3 rotateVectorAroundOrigin(const vec3& v) const ;

			vec3 getIn() const ;
			vec3 getUp() const ;
			vec3 getRight() const ;
			const real getX() const ;
			const real getY() const ;
			const real getZ() const ;
			const real getW() const ;

			Quaternion slerp(const Quaternion& p_to, const real p_time) const;

			// glRotate(theta, axis.x, axis.y, axis.z)
			void toAxisAngle(vec3* axis, Angle* theta) const;

			void lookAt(const vec3& pFrom, const vec3& pTo, const vec3& pUp);
			void lookInDirection(const vec3& pDirection, const vec3& pUp);
			void fromMatrix3(const real pMatrix[3][3]);

			bool operator==(const Quaternion& pOther) const;
			bool operator!=(const Quaternion& pOther) const;
		private:
			real m_x;
			real m_y;
			real m_z;
			real m_w;
		};
	}
}

#endif
