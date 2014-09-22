#ifndef LL_WORLD2_HPP
#define LL_WORLD2_HPP

#include <memory>

#include "lunarlady/Types.hpp"

namespace lunarlady {
	class Object2;
	class World2 {
	public:
		World2();
		~World2();

		void add(Object2* iObject);
		
		void render(real iTime);
		void update(real iTime);

	private:
		struct World2Pimpl;
		std::auto_ptr<World2Pimpl> mPimpl;
	};
}

#endif