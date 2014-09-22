#ifndef LL_LOADER_HPP
#define LL_LOADER_HPP

#include "lunarlady/Menu.hpp"
#include "lunarlady/Media.hpp"

namespace lunarlady {
	class Loader : public Menu {
	public:
		Loader(const std::string& iLoaderFile);

		void doTick(real iTime);

		real getProgress() const;
	private:
		int mLoaded;
		MediaList::iterator mCurrentMedia;
	};
}

#endif