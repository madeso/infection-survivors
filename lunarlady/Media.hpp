#ifndef LL_MEDIA_HPP
#define LL_MEDIA_HPP

#include <list>
#include "boost/utility.hpp"

namespace lunarlady {
	class Media;
	typedef std::list<Media*> MediaList;

	class Media : boost::noncopyable {
	public:
		Media();
		virtual ~Media();

		void load();
		void unload();

		virtual void doLoad() = 0;
		virtual void doUnload() = 0;

		bool isLoaded() const;

		static void UnloadAll();
		static MediaList sMedia;
	private:
		bool mIsLoaded;
	};
}

#endif