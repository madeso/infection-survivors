#include "lunarlady/File.hpp"
#include "lunarlady/FileSystem.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/Error.hpp"
#include <sstream>

namespace lunarlady {
	void WriteFile(const std::string iFileName, const char* iBuffer, std::size_t iSize) {
		PHYSFS_file* file = PHYSFS_openWrite(iFileName.c_str());
		if( file == NULL ) {
			std::ostringstream str;
			str << "Failed to write file, " << iFileName << ", reason: " << PHYSFS_getLastError();
			throw PhysFSError(str.str());
		}
		const PHYSFS_uint32 size = iSize;
		const PHYSFS_sint64 written = PHYSFS_write(file, iBuffer, sizeof(char), size );
		if( written < size ) {
			std::ostringstream str;
			str << "Failed to write enough bytes to file " << iFileName << ", " << written << " bytes written, reason: " << PHYSFS_getLastError();
			throw PhysFSError(str.str());
		}
		HandlePhysfsInitError( PHYSFS_close(file), "Failed to close file" + iFileName );
	}
	ReadFile::ReadFile(const std::string& iFileName) : mSize(0) {
		PHYSFS_file* file = PHYSFS_openRead( iFileName.c_str() );
		if( file == NULL ) {
			std::ostringstream str;
			str << "Failed to load file, " << iFileName << ", reason: " << PHYSFS_getLastError();
			throw PhysFSError(str.str());
		}
		mSize = PHYSFS_fileLength(file);
		mFile.reset( new char[mSize] );
		PHYSFS_sint64 lengthRead = PHYSFS_read (file, mFile.get(), 1, mSize);
		HandlePhysfsInitError( PHYSFS_close(file), "Failed to close file" + iFileName );
	}

	bool FileExist(const std::string iFileName) {
		return 0 != PHYSFS_exists(iFileName.c_str());
	}

	std::size_t ReadFile::getSize() const {
		return mSize;
	}
	char* ReadFile::getBuffer() const {
		return mFile.get();
	}

	void GetFileListing(const std::string& iDirectory, std::vector<std::string>* oFiles) {
		char **rc = PHYSFS_enumerateFiles( iDirectory.c_str() );

		const std::string SEPERATOR = "/";
		const bool shouldAddSeperator = !EndsWith(iDirectory, SEPERATOR);
		
		for (char **i = rc; *i != NULL; i++) {
			std::ostringstream str;
			if( shouldAddSeperator ) {
				str << iDirectory << SEPERATOR << *i;
			}
			else {
				str << iDirectory << *i;
			}
			oFiles->push_back( str.str() );
		}

		PHYSFS_freeList(rc);
	}
}