#ifndef LL_LOADED_HPP
#define LL_LOADED_HPP

#include "boost/utility.hpp"

namespace lunarlady {
	class Loaded : boost::noncopyable {
	public:
		Loaded();
		~Loaded();

		void increaseUsage();
		void decreaseUsage();
		bool isInUse() const;
	private:
		unsigned int mUsage;
	};
}

#endif