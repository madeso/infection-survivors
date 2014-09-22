#include "lunarlady/Loader.hpp"
#include "lunarlady/Script.hpp"
#include "lunarlady/Game.hpp"

namespace lunarlady {

	class GetLoadProgressScriptFunction {
	public:
		GetLoadProgressScriptFunction(Loader* iLoader) : mLoader(iLoader) {
		}
		void operator()(FunctionArgs& iArguments) {
			const real progress = mLoader->getProgress();
			Return(iArguments, progress);
		}
	private:
		Loader* mLoader;
	};

	Loader::Loader(const std::string& iLoaderFile) : Menu("loader", 400, false, false, false, iLoaderFile), mLoaded(-1) {
		RegisterFunction("getProgress", GetLoadProgressScriptFunction(this), "Gets the progress 0=started, 1=finished");
	}

	void Loader::doTick(real iTime) {
		Menu::doTick(iTime);
		EnableRender();

		if( mLoaded == -1 ) {
			mCurrentMedia = Media::sMedia.begin();
			mLoaded = 0;
		}

		const int count=1;
		for(int i=0; i<count; ++i) {
			if( mCurrentMedia == Media::sMedia.end() ) {
				disable();
				mLoaded = -1;
				return;
			}
			else {
				(*mCurrentMedia)->load();
				++mCurrentMedia;
				++mLoaded;
			}
		}
	}

	real Loader::getProgress() const {
		return (real)(mLoaded) / (real)(Media::sMedia.size());
	}
}