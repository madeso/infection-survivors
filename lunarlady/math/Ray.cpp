#include "Ray.hpp"

namespace lunarlady {
	namespace math {
		Ray::Ray(const vec3& p_start, const vec3& p_direction) : m_start(p_start), m_direction(p_direction) {
		}
		vec3 Ray::getPoint(real p_location) const {
			vec3 ret(0,0,0); getPoint(p_location, ret);
			return ret;
		}
		void Ray::getPoint(real p_location, vec3& o_result) const {
			o_result = ( m_direction * p_location) + m_start;
		}
		Ray Ray::FromTo(const vec3& p_from, const vec3& p_to) {
			return Ray(p_from, vec3(p_from, p_to));
		}
		void Ray::set(const vec3& p_from, const vec3& p_to) {
			m_start = p_from;
			m_direction = p_to - p_from;
		}
		const vec3& Ray::getStart() const {
			return m_start;
		}
		const vec3& Ray::getDirection() const {
			return m_direction;
		}
		real Ray::getClosestPointOnLine(const vec3& pPoint) const {
			const real a = m_start.getX();	const real b = m_direction.getX();	const real c = pPoint.getX();
			const real d = m_start.getY();	const real e = m_direction.getY();	const real f = pPoint.getY();
			const real g = m_start.getZ();	const real h = m_direction.getZ();	const real i = pPoint.getZ();
			const real A = b*b + e*e + h*h;
			const real B = a*b-c*b+d*e-f*e+g*h-i*h;
			return B / -A;
		}
	}
}

// closest point on line
/*In general, the closest point on an infinite line to another point, is the point that sits at the intersection between the line and a perpendicular vector to the line and passing through that point.

In short, let's have a line passing through points (A, B), and a point C. You try to find the point D that is on the line, and the closest to C.

Vector AB = (B - A);
Vector AC = (C - A);
real  t  = AB . AC / (AB . AB);
Vector D  = A + AB * t;


if you want to restrict that point to the segment [A, B],

Vector AB = (B - A);
Vector AC = (C - A);
real  t  = AB . AC / (AB . AB);
if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
Vector D  = A + AB * t;
*/