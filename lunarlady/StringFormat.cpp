#include <sstream>

#include "sgl/sgl_Assert.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Xml.hpp"
#include "lunarlady/StringFormat.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/Error.hpp"

namespace lunarlady {
	StringFormat::StringFormat(const std::string& iFileBase) {
		Assert(!sInstance, "StringFormat already created");
		sInstance = this;

		std::vector<std::string> fileNames;
		GetFileListing(iFileBase, &fileNames);
		std::size_t fileCount = fileNames.size();
		for(std::size_t fileIndex=0; fileIndex < fileCount; ++fileIndex) {
			const std::string fileName = fileNames[fileIndex];
			if( EndsWith(fileName, ".pjb")) {
				load( fileName );
			}
		}
	}
	void StringFormat::load(const std::string& iFileName) {
		ReadFile f(iFileName);
		TiXmlDocument doc( iFileName.c_str() );
		doc.Parse(f.getBuffer());
		TiXmlHandle docHandle( &doc );

		for(TiXmlElement* str = docHandle.FirstChild("strings").FirstChild("string").ToElement();
			str; str = str->NextSiblingElement() ) {
#define STRING(id) GetStringAttribute(str, "strings::string", id, iFileName)
				const std::string id = STRING("id");
				const std::string value = STRING("value");
				PrintJob job(value);
				mStringMap.insert( StringStringPair(id, job ) );
#undef STRING
		}
	}



	StringFormat::~StringFormat() {
		Assert(sInstance, "StringFormat not created in dtor");
		sInstance = 0;
	}

	StringFormat& StringFormat::GetInstance() {
		return *GetInstancePtr();
	}
	StringFormat* StringFormat::GetInstancePtr() {
		Assert(sInstance, "StringFormat isn't created");
		return sInstance;
	}

	const PrintJob& StringFormat::getString(const std::string& iType) const {
		StringStringMap::const_iterator res = mStringMap.find(iType);
		if( res == mStringMap.end() ) {
			throw MissingStringFormatError(iType);
		}
		return res->second;
	}

	StringFormat* StringFormat::sInstance = 0;
	
}