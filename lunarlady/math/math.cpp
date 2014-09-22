#include "lunarlady/math/math.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cassert>

namespace lunarlady {
	namespace math {
		const bool
		logicXor(const bool a, const bool b) {
			return a != b;
		}
		
		const real
		abs(const real a) {
			if( a > 0.0f ) return a;
			else return -a;
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		Max(const real a, const real b) {
			if( a > b ) return a;
			else return b;
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		Min(const real a, const real b) {
			if( a < b ) return a;
			else return b;
		}
		
		// ----------------------------------------------------------------------------------------------------

		const real
		floor(const real a) {
			return std::floor(a);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		ceil(const real a) {
			return std::ceil(a);
		}
		
		// ----------------------------------------------------------------------------------------------------

		const bool
		withinRange(const real a, const real b, const real min) {
			assert(min > 0.0);
			return abs(a-b) < min;
		}

		// ----------------------------------------------------------------------------------------------------

		const bool
		equal(const real a, const real b) {
			return equal5(a,b);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal0(const real a, const real b) {
			return withinRange(a, b, 1.0 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal1(const real a, const real b) {
			return withinRange(a, b, 0.1 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal2(const real a, const real b) {
			return withinRange(a, b, 0.01 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal3(const real a, const real b) {
			return withinRange(a, b, 0.001 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal4(const real a, const real b) {
			return withinRange(a, b, 0.0001 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal5(const real a, const real b) {
			return withinRange(a, b, 0.00001 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal6(const real a, const real b) {
			return withinRange(a, b, 0.000001 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const bool
		equal7(const real a, const real b) {
			return withinRange(a, b, 0.0000001 );
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		clampZero(const real a) {
			if( 0.005f > abs(a) ) {
				return 0.0f;
			}
			else {
				return a;
			}
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		limitRange(const real min, const real a, const real max) {
			if( a < min ){
				return min;
			}
			else{
				if(a>max){
					return max;
				}
				else {
					return a;
				}
			}
		}

		// ----------------------------------------------------------------------------------------------------

		const real
		wrapRange(const real min, const real a, const real max) {
			assert( min < max );
			real f = a;
			while( f > max ) f-=max-min;
			while( f < min ) f+=max-min;
			return f;
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		radToDeg(const real a) {
			return a / ONE_DEG_IN_RADIAN;
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		degToRad(const real a){
			return a * ONE_DEG_IN_RADIAN;
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		square(const real a) {
			return a*a;
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		squareRoot(const real a) {
			return sqrt(a);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		cube(const real a) {
			return a*a*a;
		}
		
		// ----------------------------------------------------------------------------------------------------

		const real
		sin(const real a) {
			return std::sin(a);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		cos(const real a) {
			return std::cos(a);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		tan(const real a) {
			return std::tan(a);
		}
		
		// ----------------------------------------------------------------------------------------------------

		const real
		asin(const real a) {
			return std::asin(a);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		acos(const real a) {
			return std::acos(a);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		atan(const real a) {
			return std::atan(a);
		}
		
		// ----------------------------------------------------------------------------------------------------
		
		const real
		interpolate(const real p_from, real p_fromOrTo, real p_to) {
			return p_from +
						( p_fromOrTo * (p_to - p_from) );
		}

		real Map01ToMultiplier(real i01, real iBase) {
			const real min = 1 / iBase;
			const real max = iBase;
			const real diff = max - min;
			return min + diff*i01;
		}
	}
}