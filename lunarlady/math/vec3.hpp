#ifndef LLM_VEC3_HPP
#define LLM_VEC3_HPP

#include "lunarlady/math/vecop.hpp"
#include "lunarlady/math/math.hpp"
#include "lunarlady/math/Angle.hpp"

namespace lunarlady {
	namespace math {

		enum V3Member {
			V3_X=0,
			V3_Y=1,
			V3_Z=2
		};

		class vec3 {
		public:
			// constructors
			vec3(real x, real y, real z) {setX(x); setY(y); setZ(z);}

			vec3(const vec3& iFrom, const vec3& iTo);

			// operators
			const vec3& operator=(const vec3& vec) { setX(vec.getX()); setY(vec.getY()); setZ(vec.getZ()); return (*this); }
			const bool operator==(const vec3& vec) const;
			const bool operator!=(const vec3& vec) const { return !(vec==(*this)); }
			const vec3 operator+() const { return (*this); }
			const vec3 operator+(const vec3& vec) const { vec3 temp(*this); temp+=vec; return temp;}
			void operator+=(const vec3& vec) { setX(getX() + vec.getX()); setY(getY()+vec.getY()); setZ(getZ()+vec.getZ()); }
			const vec3 operator-() const { return vec3( -getX(), -getY(), -getZ() ); }
			const vec3 operator-(const vec3& vec) const { vec3 temp(*this); temp-=vec; return temp; }
			void operator-=(const vec3& vec) { setX(getX()-vec.getX()); setY(getY()-vec.getY()); setZ(getZ()-vec.getZ()); }
			void operator*=(real scalar) { setX(getX()*scalar); setY(getY()*scalar); setZ(getZ()*scalar); }
			void operator/=(real scalar) { (*this)*=(1/scalar); }
			const vec3 operator*(real scalar) const { vec3 temp(*this); temp*=scalar; return temp;}
			friend inline const vec3 operator*(real scalar, const vec3& vec) { return vec*scalar; }
			const vec3 operator/(real scalar) const { vec3 temp(*this); temp/=scalar; return temp; }

			real getDotProduct(const vec3& r) const { return m[0]*r.m[0] + m[1]*r.m[1] + m[2]*r.m[2]; }
			real operator dot(const vec3& rexp) const { return getDotProduct(rexp); }

			vec3 getCrossProduct(const vec3& r) const { return vec3(getY()*r.getZ() - getZ()*r.getY(),
																	getZ()*r.getX() - getX()*r.getZ(),
																	getX()*r.getY() - getY()*r.getX()); }
			vec3 operator cross(const vec3& rexp) const { return getCrossProduct(rexp); }

			// methods
			real getLengthSquared() const { return m[0]*m[0]+m[1]*m[1]+m[2]*m[2]; }
			real getLength() const;
			vec3 getNormalized() const { vec3 temp(*this); temp.normalize(); return temp; }
			real normalize();

			bool isUnit() const;
			bool isZero() const;

			// todo: add swizzlers

			// getters
			const real getX() const { return get(V3_X); }
			const real getY() const { return get(V3_Y); }
			const real getZ() const { return get(V3_Z); }

			// setters
			void setX(real x) { set(V3_X, x); }
			void setY(real y) { set(V3_Y, y); }
			void setZ(real z) { set(V3_Z, z); }

			// low-level accessors
			const real get(V3Member member) const { return m[member]; }
			void set(V3Member member, real value) { m[member] = value; }

			const real* getArray() const {return m;}
		private:
			real m[3];
		};

		namespace op {
			class vec3 {
			public:
				real static lengthBetween(const ::lunarlady::math::vec3& from, const ::lunarlady::math::vec3& to);
				real static lengthBetweenSquared(const ::lunarlady::math::vec3& from, const ::lunarlady::math::vec3& to);

				::lunarlady::math::vec3 static getLinearInterpolation(const ::lunarlady::math::vec3& from, real value, const ::lunarlady::math::vec3& to);
				void static getLinearInterpolation(const ::lunarlady::math::vec3& from, real value, const ::lunarlady::math::vec3& to, ::lunarlady::math::vec3* out);

				real static getCosAngleBetween(const ::lunarlady::math::vec3& a, const ::lunarlady::math::vec3& b) ;
				Angle static getAngleBetween(const ::lunarlady::math::vec3& a, const ::lunarlady::math::vec3& b) ;

				static const ::lunarlady::math::vec3 origo;
				static const ::lunarlady::math::vec3 xAxisPositive;
				static const ::lunarlady::math::vec3 yAxisPositive;
				static const ::lunarlady::math::vec3 zAxisPositive;
				static const ::lunarlady::math::vec3 xAxisNegative;
				static const ::lunarlady::math::vec3 yAxisNegative;
				static const ::lunarlady::math::vec3 zAxisNegative;
			};
		};
	}
}

#endif