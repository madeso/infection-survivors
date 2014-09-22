#ifndef LL_GAME_HPP
#define LL_GAME_HPP

#include <list>
#include <string>

#include "boost/function.hpp"

#include "sgl/Key.hpp"

#include "lunarlady/Types.hpp"
#include "lunarlady/math/vec2.hpp"

namespace lunarlady {
	class State;
	class System;
	class MaterialDefinition;

	// loading routines
	class ImageLoaded; class ImageDescriptor;
	ImageLoaded* Load(const ImageDescriptor& iDescriptor);
	void Unload(ImageLoaded* iImage);

	class FontLoaded; class FontDescriptor;
	FontLoaded* Load(const FontDescriptor& iDescriptor);
	void Unload(FontLoaded* iFont);

	const std::string& GetLanguage();

	void ClearCache();

	void DisableRender();
	void EnableRender();

	const MaterialDefinition& GetMaterialDefinition(const std::string& iName);

	class Resolution {
	public:
		Resolution(int iWidth, int iHeight) : width(iWidth), height(iHeight) {
		}
		int width;
		int height;
	};

	int GetDepth();
	real GetMenuSensivity();

	typedef boost::function<State* (void)> BuildStateFunction;
	typedef boost::function<System* (void)> BuildSystemFunction;
	typedef std::pair<BuildSystemFunction, int> BuildSystemPair;

	void ExitGame();
	void UpdateStateLinks();
	void ToggleState(const std::string& iStateName);
	void DisableState(const std::string& iStateName);

	// if iIgnore is true, a state that doesn't exist doesn't trigger an assert
	// false should be used in most cases as state names should be treated as unique identifiers
	void EnableState(const std::string& iStateName, bool iIgnore=false);
	bool IsFullscreen();
	const std::string& GetFirstCommandlineArgument();
	void OnMouseMovement(const math::vec2& iMovement);
	void OnKey(const sgl::Key& iKey, bool iDown);
	void OnChar(const wchar_t iChar);

	void LoadGameConfig();

	const Resolution& GetResolution();

	class RegistratorClass {
	public:
		RegistratorClass();

		void registerState(BuildStateFunction iFunction);
		void registerSystem(BuildSystemFunction iFunction, int iPriority);
		void setupStrings(const std::string& iCompanyName, const std::string& iGameName);
		void setupLoopConstants(const real iTimeBetweenTicks, const int iMaxLoops);
		void setupAspect(const real iAspect);

		void clear();
		real getTimeBetweenTicks() const;
		int getMaxLoops() const;

		real getAspect() const;

		const std::string& getGameName() const;
		const std::string& getCompanyName() const;

		const std::string& getGeneralLanguagePath() const;

		const std::string& getGeneralStringPath() const;
		real getFontSize() const;

		const std::string& getMaterialPath() const;
	public:
		std::list<BuildStateFunction> buildStateFunctions;
		std::list<BuildSystemPair> buildSystemFunctions;

	private:
		std::string mCompanyName;
		std::string mGameName;
		std::string mGeneralStringPath;
		std::string mGeneralLanguagePath;
		std::string mMaterialPath;

		real mTimeBetweenTicks;
		int mMaxLoops;

		real mAspect;
		real mFontSize;
	};

	void SGL_main();

	RegistratorClass& Registrator();

#define LL_SYSTEM(system, priority) \
::lunarlady::System* BuildSystemFunction_ ## system () {\
	return new system (); \
} \
struct RegistringSystem_ ## system { \
	RegistringSystem_ ## system () { \
		::lunarlady::Registrator().registerSystem(& BuildSystemFunction_ ## system, priority); \
	} \
	bool uselessFunction() { \
		return this == 0; \
	} \
} RegistringSystem_ ## system ## _instance; \
bool RegistringSystem_ ## system ## _uselessFunction() { \
	return RegistringSystem_ ## system ## _instance.uselessFunction(); \
}

#define LL_STATE(state) \
	::lunarlady::State* BuildStateFunction_ ## state () {\
	return new state (); \
} \
struct RegistringState_ ## state { \
	RegistringState_ ## state () { \
		::lunarlady::Registrator().registerState(& BuildStateFunction_ ## state); \
	} \
	bool uselessFunction() { \
		return this == 0; \
	} \
} RegistringState_ ## state ## _instance; \
bool RegistringState_ ## state ## _uselessFunction() { \
	return RegistringState_ ## state ## _instance.uselessFunction(); \
}


#define LL_TITLE(companyName, gameTitle) \
struct SetupGameStrings { \
	SetupGameStrings () { \
	::lunarlady::Registrator().setupStrings(companyName, gameTitle); \
	} \
	bool uselessFunction() { \
		return this == 0; \
	} \
} SetupGameStrings_instance; \
bool SetupGameStrings_uselessFunction() { \
	return SetupGameStrings_instance.uselessFunction(); \
}

#define LL_ASPECT(aspect) \
struct SetupGameAspect { \
	SetupGameAspect () { \
	::lunarlady::Registrator().setupAspect(aspect); \
	} \
	bool uselessFunction() { \
		return this == 0; \
	} \
} SetupGameAspect_instance; \
bool SetupGameAspect_uselessFunction() { \
	return SetupGameAspect_instance.uselessFunction(); \
}

} // end of namespace

#endif