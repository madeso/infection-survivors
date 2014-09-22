#ifndef LL_POOL_HPP
#define LL_POOP_HPP

#include <map>
#include "boost/smart_ptr.hpp"
#include "boost/utility.hpp"

namespace lunarlady {
	template <class Loaded, class Descriptor>
	class Cache  : boost::noncopyable {
	public:
		Cache() {
		}

		~Cache() {
		}

		Loaded* get(const Descriptor& iDescriptor) {
			Loaded* found = find(iDescriptor);
			if( found ) {
				return IncreaseCount(found);
			}

			return IncreaseCount(loadAndAdd(iDescriptor));
		}
		void unget(Loaded* iLoaded) {
			iLoaded->decreaseUsage();
		}

		void removeUnreferencedMedia() {
			for(LoadedMap::iterator loaded=mLoadedMap.begin(); loaded != mLoadedMap.end(); ) {
				if( IsUnreferenced(*loaded) ) {
					loaded = mLoadedMap.erase(loaded);
				}
				else {
					++loaded;
				}
			}
		}

		std::size_t count() {
			return mLoadedMap.size();
		}

	private:
		static Loaded* IncreaseCount(Loaded* iLoaded) {
			iLoaded->increaseUsage();
			return iLoaded;
		}
		Loaded* find(const Descriptor& iDescriptor) {
			LoadedMap::iterator result = mLoadedMap.find(iDescriptor);
			if( result == mLoadedMap.end() ) {
				return 0;
			}
			else {
				Loaded* found = result->second.get();
				Assert(found, "Found a loaded, but it was null");
				return found;
			}
		}
		Loaded* loadAndAdd(const Descriptor& iDescriptor) {
			LoadedPtr toAdd(load(iDescriptor));
			mLoadedMap.insert( LoadedPair(iDescriptor, toAdd) );
			return toAdd.get();
		}
		Loaded* load(const Descriptor& iDescriptor) {
			return new Loaded(iDescriptor);
		}
	private:
		typedef boost::shared_ptr<Loaded> LoadedPtr;
		typedef std::map<Descriptor, LoadedPtr> LoadedMap;
		typedef std::pair<Descriptor, LoadedPtr> LoadedPair;

		static bool IsUnreferenced(const LoadedPair& iLoaded) {
			return iLoaded.second->isInUse()==false;
		}

		LoadedMap mLoadedMap;
	};
}

#endif