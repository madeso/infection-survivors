#ifndef LLM_VEC2_HPP
#define LLM_VEC2_HPP

#include "lunarlady/math/vecop.hpp"
#include "lunarlady/math/math.hpp"

namespace lunarlady {
	namespace math {

		enum V2Member {
			V2_X = 0,
			V2_Y = 1
		};

		// glsl inspired vector
		class vec2 {
		public:
			// constructors
			vec2() {setX(0); setY(0);}
			vec2(real x, real y) {setX(x); setY(y);}

			// operators
			const vec2& operator=(const vec2& vec) { setX(vec.getX()); setY(vec.getY()); return (*this); }
			const bool operator==(const vec2& vec) const;
			const bool operator!=(const vec2& vec) const { return !(vec==(*this)); }
			const vec2 operator+() const { return (*this); }
			const vec2 operator+(const vec2& vec) const { vec2 temp(*this); temp+=vec; return temp;}
			void operator+=(const vec2& vec) { setX(getX() + vec.getX()); setY(getY()+vec.getY()); }
			const vec2 operator-() const { return vec2( -getX(), -getY() ); }
			const vec2 operator-(const vec2& vec) const { vec2 temp(*this); temp-=vec; return temp; }
			void operator-=(const vec2& vec) { setX(getX()-vec.getX()); setY(getY()-vec.getY()); }
			void operator*=(real scalar) { setX(getX()*scalar); setY(getY()*scalar); }
			void operator/=(real scalar) { (*this)*=(1/scalar); }
			const vec2 operator*(real scalar) const { vec2 temp(*this); temp*=scalar; return temp;}
			friend inline const vec2 operator*(real scalar, const vec2& vec) { return vec*scalar; }
			const vec2 operator/(real scalar) const { vec2 temp(*this); temp/=scalar; return temp; }

			real getDotProduct(const vec2& r) const { return m[0]*r.m[0] + m[1]*r.m[1]; }
			real operator dot(const vec2& rexp) const { return getDotProduct(rexp); }

			// methods
			real getLengthSquared() const { return m[0]*m[0]+m[1]*m[1]; }
			real getLength() const;
			vec2 getNormalized() const { vec2 temp(*this); temp.normalize(); return temp; }
			void normalize() { (*this) /= getLength(); }

			// todo: add swizzlers

			// getters
			const real getX() const { return get(V2_X); }
			const real getY() const { return get(V2_Y); }

			// setters
			void setX(real x) { set(V2_X, x); }
			void setY(real y) { set(V2_Y, y); }

			// low-level accessors
			const real get(V2Member member) const { return m[member]; }
			void set(V2Member member, real value) { m[member] = value; }
		private:
			real m[2];
		};

		namespace op {
			namespace vec2 { 
				real lengthBetween(const ::lunarlady::math::vec2& from, const ::lunarlady::math::vec2& to);
				real lengthBetweenSquared(const ::lunarlady::math::vec2& from, const ::lunarlady::math::vec2& to);
				const ::lunarlady::math::vec2 Truncate(const ::lunarlady::math::vec2& iVector, real iMax);
			}
		}
	}
}

#endif // VEC2_HPP