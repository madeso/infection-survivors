#ifndef LLM_RANDOM_HPP
#define LLM_RANDOM_HPP

#include "lunarlady/Types.hpp"

namespace lunarlady {
	namespace math {

		// Mersenne Twister Pseudorandom number generator
		// originally developed by Takuji Nishimura and Makoto Mtsumoto
		// grabbed from Game Coding Complete by Mike McShaffry p85-87

		// period parameters
		typedef enum { CMATH_N = 624 };
		typedef enum { CMATH_M = 397 };
		typedef enum { CMATH_MATRIX_A = 0x9908b0df }; // constant vector
		typedef enum { CMATH_UPPER_MASK = 0x80000000 }; // most significant w-r bits
		typedef enum { CMATH_LOWER_MASK = 0x7fffffff }; // least significant r bits

		// tempering parameters
		typedef enum { CMATH_TEMPERING_MASK_B = 0x9d2c5680 };
		typedef enum { CMATH_TEMPERING_MASK_C = 0xefc60000 };
		
		unsigned long CMATH_TEMPERING_SHIFT_U(unsigned long y);
		unsigned long CMATH_TEMPERING_SHIFT_S(unsigned long y);
		unsigned long CMATH_TEMPERING_SHIFT_T(unsigned long y);
		unsigned long CMATH_TEMPERING_SHIFT_L(unsigned long y);

		class Random {
		public:
			Random();

			//returns a number from 0 to n, excluding n
			unsigned long random(unsigned long n);
			unsigned long random();
			int randomSign();
			real randomReal();
			real randomRealWithSign();
			double randomDouble();
			bool randomBool();
			void setRandomSeed(unsigned long n);
			unsigned long getRandomSeed();
			void randomize();
		private:
			unsigned long rseed;
			unsigned long mt[CMATH_N]; // the array for the state vector
			int mti; // mti=N+1 means mt[n] is not initialized
		};

	}
}

#endif // LLM_RANDOM_HPP