#include "physfs.h"

#include "FileSystem.hpp"

#include "lunarlady/System.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Log.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/Error.hpp"
#include "lunarlady/Language.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Xml.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/utility.hpp"

#include "sgl/sgl_Assert.hpp"

#include <memory>

namespace lunarlady {
	// ----------------------------------------------------------------------------------------------------------------------------------------
	// ------------------ CONFIG SYSTEM -------------------------------------------------------------------------------------------------------
	// ----------------------------------------------------------------------------------------------------------------------------------------

	class Config;
	namespace {
		typedef std::map<std::string, std::string> IdValueMap;
		Config* gConfig = 0;
	}

	class Config : boost::noncopyable  {
	public:
		Config() {
			Assert(!gConfig, "Only one instace of game config");
			gConfig = this;

			const std::string user = "user.cfg";
			load("default.cfg");
			if( FileExist(user) ) {
				load(user);
			}
		}
		~Config() {
			try {
				//save("user.cfg");
			} catch(...) {
				LOG1("Failed to write user config");
			}
		}

		void save(const std::string& iFileName) {
			TiXmlDocument doc(iFileName);
			TiXmlElement* root = (TiXmlElement*) doc.InsertEndChild( TiXmlElement("configurations") );

			Assert(gConfig, "No instance available, bug?");
			gConfig = 0;
			
			for(IdValueMap::const_iterator config = mConfiguration.begin();
				config != mConfiguration.end();
				++config) {
					std::vector<std::string> elements;
					SplitString(".", config->first, &elements);
					TiXmlElement* child = root;
					const std::size_t size = elements.size();
					for(std::size_t index=0; index<size; ++index) {
						const std::string subName = elements[index];
						TiXmlElement* subElement = child->FirstChildElement(subName);
						if( subElement ) {
							child = subElement;
						}
						else {
							child = (TiXmlElement*) child->InsertEndChild( TiXmlElement(subName) );
						}
					}
					SetStringText(child, config->second);
			}
			
			TiXmlPrinter printer;
			doc.Accept(&printer);
			const std::string& file = printer.Str();
			WriteFile(iFileName, file.c_str(), file.length());
		}

		void load(const std::string& iFileName) {
			ReadFile file(iFileName);
			TiXmlDocument doc(iFileName);
			doc.Parse(file.getBuffer(), 0, TIXML_ENCODING_LEGACY);
			for(TiXmlElement* child = TiXmlHandle(&doc).FirstChildElement("configurations").FirstChildElement().ToElement(); child; child = child->NextSiblingElement()) {
				handleElement(child->Value(), child);
			}
		}

		void handleElement(const std::string iMemory, TiXmlElement* iElement) {
			bool children = false;
			for(TiXmlElement* child = iElement->FirstChildElement(); child; child = child->NextSiblingElement()) {
				children = true;
				handleElement(iMemory + std::string(".") + child->Value(), child);
			}

			if( !children ) {
				const std::string name = iMemory;
				const std::string value = GetStringText(iElement);
				mConfiguration[name] = value;
			}
		}

		const std::string get(const std::string& iName) {
			IdValueMap::iterator result = mConfiguration.find(iName);
			if( result == mConfiguration.end() ) {
				throw SettingsError("Failed to find setting");
			}
			return result->second;
		}
		void set(const std::string& iName, const std::string& iValue) {
			if( mConfiguration.find(iName) == mConfiguration.end() ) {
				throw SettingsError("Failed to find setting");
			}
			mConfiguration[iName]=iValue;
		}

		template<class T>
		T get(const std::string& iName) {
			return boost::lexical_cast<T>(get(iName));
		}
		template<class T>
		void set(const std::string& iName, const T& iValue) {
			return set(iName, boost::lexical_cast<std::string>(iValue));
		}

		void step(real iTime) {
		}

		IdValueMap mConfiguration;
	};

	int GetInt(const std::string& iName) {
		Assert(gConfig, "No instance available, bug?");
		return gConfig->get<int>(iName);
	}
	real GetReal(const std::string& iName) {
		Assert(gConfig, "No instance available, bug?");
		return gConfig->get<real>(iName);
	}
	bool GetBool(const std::string& iName) {
		Assert(gConfig, "No instance available, bug?");
		return gConfig->get<int>(iName)==1;
	}
	const std::string GetString(const std::string& iName) {
		Assert(gConfig, "No instance available, bug?");
		return gConfig->get(iName);
	}

	void SetInt(const std::string& iName, int iValue) {
		Assert(gConfig, "No instance available, bug?");
		gConfig->set(iName, iValue);
	}
	void SetReal(const std::string& iName, real iValue)  {
		Assert(gConfig, "No instance available, bug?");
		gConfig->set(iName, iValue);
	}
	void SetBool(const std::string& iName, bool iValue)  {
		Assert(gConfig, "No instance available, bug?");
		int value = iValue ? 1 : 0;
		gConfig->set(iName, value);
	}
	void SetString(const std::string& iName, const std::string& iValue)  {
		Assert(gConfig, "No instance available, bug?");
		gConfig->set(iName, iValue);
	}

	// ----------------------------------------------------------------------------------------------------------------------------------------
	// ------------------ FILE SYSTEM ---------------------------------------------------------------------------------------------------------
	// ----------------------------------------------------------------------------------------------------------------------------------------
	namespace {
		bool RemoveInvalidCharacters(char c) {
			if( c == ' ' ) return true;
			if( c == ':' ) return true;
			if( c == '.' ) return true;
			if( c == '/' ) return true;
			if( c == '\\' ) return true;
			if( c == ':' ) return true;
			return false;
		}
	}

	void HandlePhysfsInitError(int iOk, const std::string& iDescription) {
		if( iOk == 0 ) {
			std::ostringstream str;
			str << iDescription << ": " << PHYSFS_getLastError();
			LOG1( "PhysFS: " << str.str() );
			throw PhysFSError(str.str());
		}
	}

	class FileSystem : public System {
	public:
		FileSystem() : System("File") {
			HandlePhysfsInitError( PHYSFS_init( GetFirstCommandlineArgument().c_str() ), "Failed to init");
			std::string gameName = Registrator().getGameName();
			gameName.erase( std::remove_if(gameName.begin(), gameName.end(), &RemoveInvalidCharacters),
				gameName.end()
				);
			HandlePhysfsInitError(
				PHYSFS_setSaneConfig(Registrator().getCompanyName().c_str(), gameName.c_str(), 0, 0, 0),
				"Failed to set sane config");
			add("..");
			mConfig.reset( new Config() );
			LoadGameConfig();
			add( std::string("../language/") + GetLanguage() + "/");
		}
		~FileSystem() {
			mConfig.reset();
			int ok = PHYSFS_deinit();
			if( ok==0 ) {
				LOG1( "PhysFS error on exit: " << PHYSFS_getLastError() );
			}
		}

		void add(const std::string& iPath) {
			const std::string message = "Failed to add ";
			HandlePhysfsInitError( PHYSFS_addToSearchPath(iPath.c_str(), 1), message + iPath + " to searchpath");
		}

		void step(real iTime) {
		}

		std::auto_ptr<Config> mConfig;
	};

	LL_SYSTEM(FileSystem, 000);
}