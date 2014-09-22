#ifndef LL_STRING_FORMAT_HPP
#define LL_STRING_FORMAT_HPP

#include <map>

#include "lunarlady/PrintJob.hpp"
#include "boost/utility.hpp"

namespace lunarlady {
	class StringFormat : boost::noncopyable {
	public:
		StringFormat(const std::string& iFileBase);
		~StringFormat();

		static StringFormat& GetInstance();
		static StringFormat* GetInstancePtr();

		const PrintJob& getString(const std::string& iType) const;
	protected:
		void load(const std::string& iFileName);
	private:
		typedef std::map<std::string, PrintJob> StringStringMap;
		typedef std::pair<std::string, PrintJob> StringStringPair;
		StringStringMap mStringMap;

		static StringFormat* sInstance;
	};
}

#endif