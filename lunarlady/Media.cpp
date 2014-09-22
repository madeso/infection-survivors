#include "lunarlady/Media.hpp"
#include "lunarlady/Game.hpp"

namespace lunarlady {
	MediaList Media::sMedia;

	Media::Media() : mIsLoaded(false) {
		sMedia.push_back(this);
		EnableState("loader", true);
	}
	Media::~Media() {
		sMedia.remove(this);
	}

	void Media::load() {
		doLoad();
		mIsLoaded = true;
	}
	void Media::unload() {
		doUnload();
		mIsLoaded = false;
	}
	
	bool Media::isLoaded() const {
		return mIsLoaded;
	}

	void Media::UnloadAll() {
		for(MediaList::iterator media = sMedia.begin(); media != sMedia.end(); ++media) {
			(*media)->unload();
		}
	}
}