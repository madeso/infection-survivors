#ifndef LL_TEMPLATED_MEDIA_HPP
#define LL_TEMPLATED_MEDIA_HPP

#include "boost/smart_ptr.hpp"
#include "lunarlady/Media.hpp"
#include "sgl/sgl_Assert.hpp"

namespace lunarlady {
	template<class Loaded, class Descriptor>
	class TemplatedMedia : public Media {
	public:
		TemplatedMedia(const Descriptor& iDescriptor) : mDescriptor(iDescriptor), mLoaded(0) {
		}
		~TemplatedMedia() {
			unloadMedia();
		}

		void doLoad() {
			unload();
			mLoaded = Loaded::Load(mDescriptor);
		}

		void doUnload() {
			unloadMedia();
		}

		Loaded* operator->() {
			Assert( isLoaded(), "TemplatedMedia must be loaded");
			Assert( mLoaded, "Media error, media says loaded, but data is empty");
			return mLoaded;
		}

		Loaded& get() {
			Assert( isLoaded(), "TemplatedMedia must be loaded");
			Assert( mLoaded, "Media error, media says loaded, but data is empty");
			return *mLoaded;
		}
	private:
		void unloadMedia() {
			if( mLoaded ) {
				Loaded::Unload(mLoaded);
				mLoaded = 0;
			}
		}

		Descriptor mDescriptor;
		Loaded* mLoaded;
	};
}

#endif