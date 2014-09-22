#include "lunarlady/Language.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/System.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Xml.hpp"
#include "lunarlady/Error.hpp"
#include "lunarlady/Script.hpp"

#include <map>

#include "sgl/sgl_Assert.hpp"

namespace lunarlady {
	namespace {
		typedef std::map<std::string, std::wstring> LanguageMap;
		typedef std::pair<std::string, std::wstring> LanguagePair;

		void LoadLanguageFile(LanguageMap* iLanguageMap, const std::string iFileName) {
			ReadFile file(iFileName);
			TiXmlDocument doc(iFileName.c_str());
			doc.Parse(file.getBuffer(), 0, TIXML_ENCODING_UTF8);
			TiXmlHandle docHandle( &doc );

			for(TiXmlElement* str = docHandle.FirstChild("language").FirstChild("string").ToElement();
				str; str = str->NextSiblingElement("string") ) {
					const std::wstring longId = GetWStringAttribute(str, "language::string", "id", iFileName);
					const std::wstring value = GetWStringAttribute(str, "language::string", "value", iFileName);
					const std::string id(longId.begin(), longId.end());
					iLanguageMap->insert( LanguagePair(id, value) );
			}
		}

		void LoadLanguage(LanguageMap* iLanguageMap, const std::string iBaseDir) {
			iLanguageMap->clear();
			std::vector<std::string> fileNames;
			GetFileListing(iBaseDir, &fileNames);
			const std::size_t fileCount = fileNames.size();
			for(std::size_t fileIndex=0; fileIndex<fileCount; ++fileIndex) {
				const std::string fileName = fileNames[fileIndex];
				LoadLanguageFile(iLanguageMap, fileName);
			}
		}
	}
	class Language {
	public:
		Language(const std::string iLanguagePath) : mLanguagePath(iLanguagePath) {
			Assert(!sInstance, "Already has a language instance");
			sInstance = this;

			reloadAll();
			loadLanguageNames("languages.xml");
		}

		~Language() {
			Assert(sInstance, "No language instance exist, bug?");
			sInstance = 0;
		}

		std::wstring getString(const std::string& iString) {
			LanguageMap::const_iterator res;

#define TRY(x)	if( ( res=x.find(iString) )==x.end() )
			TRY(mBase)
				TRY(mLevel)
					throw MissingStringError(iString);
#undef TRY
			return res->second;
		}

		void loadLevel(const std::string iPath) {
			if( mLevelPath == iPath ) return;
			mLevelPath = iPath;
			reloadLevel();
		}

		static Language& GetInstance() {
			return *GetInstancePtr();
		}
		static Language* GetInstancePtr() {
			Assert(sInstance, "Need an language instance");
			return sInstance;
		}

		typedef std::map<std::string, std::string> LanguageIdNameMap;
		LanguageIdNameMap mLanguageIdNameMap;

		void loadLanguageNames(const std::string& iFileName) {
			ReadFile file(iFileName);
			TiXmlDocument doc(iFileName.c_str());
			doc.Parse(file.getBuffer(), 0, TIXML_ENCODING_UTF8);
			TiXmlHandle docHandle( &doc );

			for(TiXmlElement* str = docHandle.FirstChild("languages").FirstChild("language").ToElement();
				str; str = str->NextSiblingElement("language") ) {
					const std::string id = GetStringAttribute(str, "languages::language", "id", iFileName);
					const std::string name = GetStringText(str);
					mLanguageIdNameMap.insert( LanguageIdNameMap::value_type(id, name) );
			}
		}

		std::string getNextLanguage(const std::string& iLanguage) {
			LanguageIdNameMap::iterator result = mLanguageIdNameMap.find(iLanguage);
			if( result != mLanguageIdNameMap.end() ) {
				++result;
			}
			if( result == mLanguageIdNameMap.end() ) {
				return mLanguageIdNameMap.begin()->first;
			}
			else {
				return result->first;
			}
		}
		std::string getPreviousLanguage(const std::string& iLanguage) {
			LanguageIdNameMap::iterator result = mLanguageIdNameMap.find(iLanguage);
			if( result != mLanguageIdNameMap.end() ) {
				--result;
			}
			if( result == mLanguageIdNameMap.end() ) {
				return mLanguageIdNameMap.rbegin()->first;
			}
			else {
				return result->first;
			}
		}
		std::string getFullLanguageName(const std::string& iLanguage) {
			LanguageIdNameMap::iterator result = mLanguageIdNameMap.find(iLanguage);
			if( result != mLanguageIdNameMap.end() ) {
				return result->second;
			}
			else {
				return "???";
			}
		}

	private:
		void reloadAll() {
			LoadLanguage(&mBase, mLanguagePath);

			reloadLevel();
		}

		void reloadLevel() {
			if( !mLevelPath.empty() ) {
				LoadLanguage(&mLevel,  mLevelPath);
			}
		}

		LanguageMap mBase;
		LanguageMap mLevel;

		std::string mLevelPath;
		const std::string mLanguagePath;

		static Language* sInstance;
	};
	Language* Language::sInstance = 0;

	std::wstring String(const std::string& iString) {
		return Language::GetInstance().getString(iString);
	}
	void LanguageLoadLevel(const std::string& iPath) {
		Language::GetInstance().loadLevel(iPath);
	}

	void GetFullLanguageNameScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "Function need 1 language name id");
		}
		ArgVarString(language, iArgs, 0);
		Return(iArgs, Language::GetInstance().getFullLanguageName(language));
	}
	SCRIPT_FUNCTION(getFullLanguageName, GetFullLanguageNameScriptFunction, "returns true if fullscreen, false otherwise");

	void GetPreviousLanguageScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "Function need 1 language name id");
		}
		ArgVarString(language, iArgs, 0);
		Return(iArgs, Language::GetInstance().getPreviousLanguage(language));
	}
	SCRIPT_FUNCTION(getPreviousLanguage, GetPreviousLanguageScriptFunction, "grabs the previous languag, wants the current language");

	void GetNextLanguageScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "Function need 1 language name id");
		}
		ArgVarString(language, iArgs, 0);
		Return(iArgs, Language::GetInstance().getNextLanguage(language));
	}
	SCRIPT_FUNCTION(getNextLanguage, GetNextLanguageScriptFunction, "grabs the nexst language, wants the current language");

	class LanguageSystem : public System {
	public:
		LanguageSystem() : System("Language"), mLanguage( Registrator().getGeneralLanguagePath() ) {
		}
		~LanguageSystem() {
		}

		void step(real iTime) {
		}

		Language mLanguage;
	};

	LL_SYSTEM(LanguageSystem, 500);
}