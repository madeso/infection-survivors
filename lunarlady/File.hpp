#ifndef LL_FILE_HPP
#define LL_FILE_HPP

#include <string>
#include <vector>
#include "boost/smart_ptr.hpp"
#include "boost/utility.hpp"
#include "physfs.h"

namespace lunarlady {

	void GetFileListing(const std::string& iDirectory, std::vector<std::string>* oFiles);
	bool FileExist(const std::string iFileName);
	void WriteFile(const std::string iFileName, const char* iBuffer, std::size_t iSize);

	class ReadFile : boost::noncopyable {
	public:
		ReadFile(const std::string& iFileName);

		std::size_t getSize() const;
		char* getBuffer() const;
	private:
		boost::scoped_array<char> mFile;
		PHYSFS_sint64 mSize;
	};
}

#endif