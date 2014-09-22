#ifndef UTIL_STRINGMAP_HPP
#define UTIL_STRINGMAP_HPP

namespace util {
	template <class String, class Char>
	class StringNode {
	public:
		StringNode() : mEnd(false) {
		}
		void insert(const String& iToInsert, const std::size_t iIndex, const std::size_t iLength) {
			if(iLength > 0 && iIndex<iLength ) {
				const Char c = iToInsert[iIndex];
				mMap[c].insert(iToInsert, iIndex+1, iLength);
			}
			else {
				//if( iIndex == iLength-1 ) {
				mEnd = true;
				//}
			}
		}

		bool has(const String& iToFind, const std::size_t iIndex, const std::size_t iLength) {
			if( mEnd && iIndex>=iLength) {
				return true;
			}

			if( iLength > 0 && iIndex<iLength ) {
				const Char c = iToFind[iIndex];
				return mMap[c].has(iToFind, iIndex+1, iLength);
			}
			else {
				return false;
			}
		}

		template <class Container>
		void find(const String& iToFind, const std::size_t iIndex, const std::size_t iLength, const String& iHistory, Container* oContainer) {
			if( mEnd && iIndex>=iLength) {
				oContainer->push_back(iHistory);
			}

			if( iLength > 0 && iIndex<iLength ) {
				const Char c = iToFind[iIndex];
				mMap[c].find(iToFind, iIndex+1, iLength, iHistory + c, oContainer);
			}
			else {
				for(StringNodeMap::iterator i=mMap.begin(); i!=mMap.end(); ++i) {
					const Char c = i->first;
					i->second.find(iToFind, iIndex+1, iLength, iHistory + c, oContainer);
				}
			}
		}
	private:
		typedef std::map<Char, StringNode<String, Char> > StringNodeMap;
		StringNodeMap mMap;
		bool mEnd;
	};

	template <class String, class Char>
	class StringMap {
	public:
		explicit StringMap(const String& iStartString) : mStartString(iStartString) {
		}
		void insert(const String& iToInsert) {
			mRoot.insert(iToInsert, 0, iToInsert.length());
		}
		template <class Container>
		void find(const String& iToFind, Container* oContainer) {
			mRoot.find(iToFind, 0, iToFind.length(), mStartString, oContainer);
		}

		bool has(const String& iToFind) {
			return mRoot.has(iToFind, 0, iToFind.length());
		}
	private:
		StringNode<String, Char> mRoot;
		const String mStartString;
	};
}

#endif