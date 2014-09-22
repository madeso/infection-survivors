#include <list>
#include <vector>
#include <algorithm>

#include "sgl/sgl.hpp"
#include "boost/utility.hpp"

#include "lunarlady/math/Math.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/State.hpp"
#include "lunarlady/System.hpp"
#include "lunarlady/Log.hpp"
#include "lunarlady/OpenGL_util.hpp"
#include "lunarlady/Script.hpp"
#include "lunarlady/Message.hpp"
#include "lunarlady/Display.hpp"

#include "lunarlady/Cache.hpp"
#include "lunarlady/Image.hpp"
#include "lunarlady/Font.hpp"

#include "lunarlady/Config.hpp"

#include "lunarlady/MaterialDefinitionContainer.hpp"
#include "lunarlady/MaterialDefinition.hpp"

namespace lunarlady {

	#define MakeSystemHappy(system) extern bool RegistringSystem_ ## system ## _uselessFunction()
	#define KeepSystemHappy(system) RegistringSystem_ ## system ## _uselessFunction()

	#define MakeStateHappy(system) extern bool RegistringState_ ## system ## _uselessFunction()
	#define KeepStateHappy(system) RegistringState_ ## system ## _uselessFunction()

	MakeSystemHappy(SglSystem);
	MakeSystemHappy(OpenglSystem);
	MakeSystemHappy(FileSystem);
	MakeSystemHappy(DevilSystem);
	MakeSystemHappy(StringFormatSystem);
	MakeSystemHappy(SoundSystem);

	MakeStateHappy(ConsoleState);

	void KeepCompilerFromRemovingClasses() {
		KeepSystemHappy(SglSystem);
		KeepSystemHappy(OpenglSystem);
		KeepSystemHappy(FileSystem);
		KeepSystemHappy(DevilSystem);
		KeepSystemHappy(StringFormatSystem);
		KeepSystemHappy(SoundSystem);

		KeepStateHappy(ConsoleState);
	}

	RegistratorClass& Registrator() {
		static RegistratorClass rc;
		return rc;
	}

	const std::string& RegistratorClass::getGameName() const {
		return mGameName;
	}
	const std::string& RegistratorClass::getCompanyName() const {
		return mCompanyName;
	}

	void RegistratorClass::clear() {
		buildStateFunctions.clear();
		buildSystemFunctions.clear();
	}
	real RegistratorClass::getTimeBetweenTicks() const {
		return mTimeBetweenTicks;
	}
	int RegistratorClass::getMaxLoops() const {
		return mMaxLoops;
	}

	real RegistratorClass::getAspect() const {
		return mAspect;
	}

	RegistratorClass::RegistratorClass() : mCompanyName("Unknown company"), mGameName("Unknown game"), mGeneralStringPath("printjobs/"), mGeneralLanguagePath("strings/"), mMaterialPath("materials/"), mTimeBetweenTicks(30/1000.0), mMaxLoops(3), mAspect(14.0/9.0), mFontSize(0.40) {
	}

	void RegistratorClass::registerState(BuildStateFunction iFunction) {
		buildStateFunctions.push_back(iFunction);
	}

	void RegistratorClass::setupStrings(const std::string& iCompanyName, const std::string& iGameName) {
		mCompanyName = iCompanyName;
		mGameName = iGameName;
	}

	void RegistratorClass::setupAspect(const real iAspect) {
		mAspect = iAspect;
	}

	const std::string& RegistratorClass::getGeneralLanguagePath() const {
		return mGeneralLanguagePath;
	}

	const std::string& RegistratorClass::getMaterialPath() const {
		return mMaterialPath;
	}

	const std::string& RegistratorClass::getGeneralStringPath() const {
		return mGeneralStringPath;
	}

	real RegistratorClass::getFontSize() const {
		return mFontSize;
	}

	void RegistratorClass::setupLoopConstants(const real iTimeBetweenTicks, const int iMaxLoops) {
		mTimeBetweenTicks = iTimeBetweenTicks;
		mMaxLoops = iMaxLoops;
	}

	void RegistratorClass::registerSystem(BuildSystemFunction iFunction, int iPriority) {
		buildSystemFunctions.push_back( BuildSystemPair(iFunction, iPriority) );

		struct SortSystems {
			static bool AccordingToPriority(const BuildSystemPair& lhs, const BuildSystemPair& rhs) {
				return lhs.second < rhs.second;
			}
		};
		buildSystemFunctions.sort(&SortSystems::AccordingToPriority);
		//std::sort(buildSystemFunctions.begin(), buildSystemFunctions.end(), &SortSystems::AccordingToPriority);
	}

	struct FindState {
		FindState(const std::string& iStateName, StatePtr** oState) : mStateName(iStateName), mState(oState) {
		}
		void operator() (StatePtr& iState) {
			if( iState->getName() == mStateName ) {
				*mState = &iState;
			}
		}
		StatePtr** mState;
		const std::string& mStateName;
	};

	class Game : boost::noncopyable {
	public:
		Game(const std::string& iFirstCommandlineArgument) : mRunning(false), mTopState(0), mFirstCommandlineArgument(iFirstCommandlineArgument), mDepth(0), mMenuMouseSensivity(0.5), mRenderDisabled(false), mLanguage("en"),
#ifdef NDEBUG
			mIsFullscreen(true), mResolution(1280, 1024)
#else
			mIsFullscreen(false), mResolution(800, 600)
#endif
			{
			Assert(!sInstance, "Already have an game instance");
			sInstance = this;
		}
		~Game() {
			struct Kill {
				static void Systems(SystemPtr& iSystem) {
					iSystem.reset();
				}
				static void States(StatePtr& iState) {
					iState.reset();
				}
			};
			std::for_each(mStates.rbegin(), mStates.rend(), &Kill::States);
			std::for_each(mSystems.rbegin(), mSystems.rend(), &Kill::Systems);

			fontCache.removeUnreferencedMedia();
			imageCache.removeUnreferencedMedia();

			Assert(fontCache.count()==0, "Not all fonts were cleared");
			Assert(imageCache.count()==0, "Not all images were cleared");

			Assert(sInstance, "Instance doesn't exist, bug?");
			sInstance = 0;
		}

		void addState(State* iState) {
			StatePtr state(iState);
			mStates.push_back(state);
			updateStates();
		}
		void addSystem(System* iSystem) {
			SystemPtr system(iSystem);
			mSystems.push_back(system);
		}

		void exitGame() {
			mRunning = false;
		}

		const Resolution& getResolution() const {
			return mResolution;
		}
		
		void updateStateLinks() {
			setupStateLinks();
		}

		bool isFullscreen() const {
			return mIsFullscreen;
		}

		void setFullscreen(bool iFullscreen) {
			mIsFullscreen = iFullscreen;
		}

		void onMouseMovement(const math::vec2& iMovement) {
			if( mTopState ) {
				mTopState->onMouseMovement(iMovement);
			}
		}
		void onKey(const sgl::Key& iKey, bool iDown) {
			if( mTopState ) {
				mTopState->onKey(iKey, iDown);
			}
		}

		void onChar(const wchar_t iChar) {
			if( mTopState ) {
				mTopState->onChar(iChar);
			}
		}

		const std::string& getFirstCommandlineArgument() const {
			return mFirstCommandlineArgument;
		}

		void enableState(const std::string& iStateName, bool iIgnore) {
			StatePtr* result = 0;
			std::for_each(mStates.begin(), mStates.end(), FindState(iStateName, &result) );
			if( !result && iIgnore ) return;
			Assert(result, "Failed to find a state by that name");
			StatePtr& theState = *result;
			theState->setEnabled(true);
		}

		void disableState(const std::string& iStateName) {
			StatePtr* result = 0;
			std::for_each(mStates.begin(), mStates.end(), FindState(iStateName, &result) );
			Assert(result, "Failed to find a state by that name");
			StatePtr& theState = *result;
			theState->setEnabled(false);
		}

		void toggleState(const std::string& iStateName) {
			StatePtr* result = 0;
			std::for_each(mStates.begin(), mStates.end(), FindState(iStateName, &result) );
			Assert(result, "Failed to find a state by that name");
			StatePtr& theState = *result;
			const bool state = theState->isEnabled();
			theState->setEnabled(!state);
		}

		void loop() {
			loop( Registrator().getTimeBetweenTicks(), Registrator().getMaxLoops() );
		}

		const MaterialDefinition& getMaterialDefinition(const std::string& iName) const {
			return mMaterialDefinitionContainer->getMaterialDefinition(iName);
		}

		void loop(real iTimeBetweenTicks, int iMaxLoops) {
			init();
			real time0;
			real time1;
			real frameTime;

			real latestTickTime = 0;
			real latestFrameTime = 0;
			real latestRenderTime = 0;

			mRunning = true;
		
			LOG1( "--- Main loop: started ---" );
		
			time0 = getTime();
			do {
				time1 = getTime();
				frameTime = 0;
				int numLoops = 0;
		
				/*
					The concept behind this is forcing every game logic tick to represent a
					fixed amount of real-time (real-time is the time shown on your wristwatch).
					Of course, ticks better take less CPU time to execute, than the real-time
					it represents.
				*/
				while((time1-time0) > iTimeBetweenTicks && numLoops < iMaxLoops) {
					real start = getTime();
					tick(iTimeBetweenTicks);
					latestTickTime = getTime() - start;
					time0 += iTimeBetweenTicks;
					frameTime += iTimeBetweenTicks;
					numLoops++;
					// Could this be a good idea? We're not doing it, anyway.
					// time1 = getTime();
				}
		
				{
					real start = getTime();
					const real delta = time1-time0;
					frame(delta);
					latestFrameTime = getTime() - start;
				}
		
				// if playing solo and game logic takes way too long, discard pending time
				/*if( !mIsNetworkGame && (time1-time0) > iTimeBetweenTicks ) {
					time0 = time1 - iTickTime;
				}*/
		
				//if(mCanRender) {
				{
					real percentWithinTick = ::lunarlady::math::Min(1.0, time1-time0/iTimeBetweenTicks);
					real start = getTime();
					render(percentWithinTick);
					latestRenderTime = getTime() - start;
				}

				// i could have send theese to the draw func, but then again, the values wouldn't be in sync
				//   i.e. the render time would always be one frame after the other values
				//   (frame time to the point to be more on the point)
				const int precision=3;
				DisplayPrecision("tick", latestTickTime, precision, latestTickTime<iTimeBetweenTicks);
				DisplayPrecision("frame", latestFrameTime, precision, true);
				DisplayPrecision("render", latestRenderTime, precision, true);

				//}
			} while ( shouldRun() );
			LOG1( "--- Main loop: ended -----" );

			saveConfig();
			deinit();
		}

		void loadConfig() {
			mResolution.height = GetInt("display.resolution.height");
			mResolution.width = GetInt("display.resolution.width");
			mIsFullscreen = GetBool("display.fullscreen");
			mDepth = GetInt("display.depth");
			mMenuMouseSensivity = GetReal("menu.mouse.sensivity");
			mLanguage = GetString("language");
		}

		void saveConfig() {
			SetInt("display.resolution.height", mResolution.height);
			SetInt("display.resolution.width", mResolution.width);
			SetBool("display.fullscreen", mIsFullscreen);
			SetInt("display.depth", mDepth);
			SetReal("menu.mouse.sensivity", mMenuMouseSensivity);
			SetString("language", mLanguage);
		}

		int getDepth() const {
			return mDepth;
		}

		const std::string& getLanguage() {
			return mLanguage;
		}
		void setLanguage(const std::string& iLanguage) {
			mLanguage = iLanguage;
		}

		real getMenuSensivity() const {
			return mMenuMouseSensivity;
		}
		
		void disableRender() {
			mRenderDisabled = true;
		}
		void enableRender() {
			mRenderDisabled = false;
		}

		static Game* GetInstancePtr() {
			Assert(sInstance, "Instance doesnt exist yet");
			return sInstance;
		}
		static Game& GetInstance() {
			return *GetInstancePtr();
		}

		Cache<ImageLoaded, ImageDescriptor> imageCache;
		Cache<FontLoaded, FontDescriptor> fontCache;
		
	private:
		// private functions
		void init() {
			KeepCompilerFromRemovingClasses();
			buildSystems();
			loadMaterials();
			displayLogos();
			buildStates();
		}

		void loadMaterials() {
			mMaterialDefinitionContainer.reset(new MaterialDefinitionContainer(Registrator().getMaterialPath()) );
		}

		void displayLogos() {
			// IMPLEMENT ME
			// own loop and rendering
		}

		void buildStates() {
			struct SetupStates {
				SetupStates(Game* iGame) : mGame(iGame) {
				}
				void operator() (BuildStateFunction& iFunction) {
					State* state = iFunction();
					mGame->addState(state);
				}
				Game* mGame;
			};

			std::for_each(Registrator().buildStateFunctions.begin(), Registrator().buildStateFunctions.end(), SetupStates(this) );
		}

		void frame(real iTime) {
			if( !shouldRun() ) {
				return;
			}
			stepSystems(iTime);
			if( !shouldRun() ) {
				return;
			}
			if( mTopState ) {
				mTopState->frame(iTime);
			}
		}
		void tick(real iTime) {
			if( !shouldRun() ) {
				return;
			}
			if( mTopState ) {
				mTopState->tick(iTime);
			}
		}
		void render(real iTime) {
			if( mRenderDisabled ) {
				return;
			}
			if( !shouldRun() ) {
				return;
			}
			preRender();
			if( mTopState ) {
				mTopState->render(iTime);
			}
			postRender();
		}
		bool shouldRun() {
			return mRunning;
		}

		void stepSystems(real iTime) {
			struct StepSystems {
				StepSystems (real iTime) : mTime(iTime) {
				}
				void operator() (SystemPtr& iSystem) {
					iSystem->step(mTime);
				}

				real mTime;
			};

			std::for_each(mSystems.begin(), mSystems.end(), StepSystems(iTime) );
		}

		void buildSystems() {
			struct SystemBuilder {
				SystemBuilder(Game* iGame) : mGame(iGame) {
				}
				void operator()(BuildSystemPair& iFunction) {
					mGame->addSystem(iFunction.first());
				}
				Game* mGame;
			};
			std::for_each(Registrator().buildSystemFunctions.begin(), Registrator().buildSystemFunctions.end(), SystemBuilder(this) );
		}
		void preRender() {
			DrawBorders();
		}
		void postRender() {
			sgl::SwapBuffers();
		}
		void deinit() {
		}
		real getTime() {
			return sgl::GetTicks() / sgl::GetTicksPerSecond();
		}

		void sortStatesAccordingToPriority() {
			struct SortStates {
				static bool AccordingToPriority(const StatePtr& lhs, const StatePtr& rhs) {
					return lhs->getPriority() > rhs->getPriority();
				}
			};
			mStates.sort(&SortStates::AccordingToPriority);
			//std::sort(mStates.begin(), mStates.end(), &SortStates::AccordingToPriority);
		}

		void setupStateLinks() {
			State* previousState = 0;
			mTopState = 0;

			struct SetupLinks {
				SetupLinks(State** iTopState, State** iPreviousState) : mTopState(iTopState), mPreviousState(iPreviousState) {
				}
				void operator() (StatePtr& iState) {
					iState->clearStateLinks();
					if( iState->isEnabled() ) {
						State* currentState = iState.get();
						if( *mPreviousState ) {
							(*mPreviousState)->setPreviousState(currentState);
							currentState->setNextState(*mPreviousState);
						}
						if( *mTopState ) {
						}
						else {
							*mTopState = currentState;
						}

						(*mPreviousState) = currentState;
					}
				}
				State** mPreviousState;
				State** mTopState;
			};

			std::for_each(mStates.begin(), mStates.end(),  SetupLinks(&mTopState, &previousState) );

			if( mTopState ) {
				if( mTopState ) {
					mTopState->focus(true);
				}
			}
		}

		void updateStates() {
			sortStatesAccordingToPriority();
			setupStateLinks();
		}

	private:
		// member variable
		std::list< StatePtr > mStates;
		std::list< SystemPtr > mSystems;
		State* mTopState;
		bool mRunning;
		Resolution mResolution;
		bool mIsFullscreen;
		int mDepth;
		real mMenuMouseSensivity;
		bool mRenderDisabled;
		std::string mLanguage;

		const std::string mFirstCommandlineArgument;
		std::auto_ptr<MaterialDefinitionContainer> mMaterialDefinitionContainer;

		static Game* sInstance;
	};
	Game* Game::sInstance = 0;

	void ExitGame() {
		Game::GetInstance().exitGame();
	}

	void EnableState(const std::string& iStateName, bool iIgnore) {
		Game::GetInstance().enableState(iStateName, iIgnore);
	}

	void ToggleState(const std::string& iStateName) {
		Game::GetInstance().toggleState(iStateName);
	}
	void DisableState(const std::string& iStateName) {
		Game::GetInstance().disableState(iStateName);
	}

	const Resolution& GetResolution() {
		return Game::GetInstance().getResolution();
	}

	void UpdateStateLinks() {
		Game::GetInstance().updateStateLinks();
	}

	bool IsFullscreen() {
		return Game::GetInstance().isFullscreen();
	}

	const std::string& GetFirstCommandlineArgument() {
		return Game::GetInstance().getFirstCommandlineArgument();
	}

	void OnMouseMovement(const math::vec2& iMovement) {
		Game::GetInstance().onMouseMovement(iMovement);
	}
	void OnKey(const sgl::Key& iKey, bool iDown) {
		Game::GetInstance().onKey(iKey, iDown);
	}
	void OnChar(const wchar_t iChar) {
		Game::GetInstance().onChar(iChar);
	}

	void ClearCache() {
		Game::GetInstance().fontCache.removeUnreferencedMedia();
		Game::GetInstance().imageCache.removeUnreferencedMedia();
	}

	ImageLoaded* Load(const ImageDescriptor& iDescriptor){
		return Game::GetInstance().imageCache.get(iDescriptor);
	}
	void Unload(ImageLoaded* iImage) {
		Game::GetInstance().imageCache.unget(iImage);
	}

	FontLoaded* Load(const FontDescriptor& iDescriptor) {
		return Game::GetInstance().fontCache.get(iDescriptor);
	}
	void Unload(FontLoaded* iFont) {
		Game::GetInstance().fontCache.unget(iFont);
	}

	const std::string& GetLanguage() {
		return Game::GetInstance().getLanguage();
	}

	const MaterialDefinition& GetMaterialDefinition(const std::string& iName) {
		return Game::GetInstance().getMaterialDefinition(iName);
	}

	void LoadGameConfig() {
		Game::GetInstance().loadConfig();
	}
	int GetDepth() {
		return Game::GetInstance().getDepth();
	}

	real GetMenuSensivity() {
		return Game::GetInstance().getMenuSensivity();
	}
	void DisableRender() {
		Game::GetInstance().disableRender();
	}
	void EnableRender() {
		Game::GetInstance().enableRender();
	}

	// ----------------------------------------------------------------------
	// Exit game script function
	void ExitGameScriptFunction(FunctionArgs& iArgs) {
		Game::GetInstance().exitGame();
	}
	SCRIPT_FUNCTION(exit, ExitGameScriptFunction, "Exits the game");

	// ----------------------------------------------------------------------
	// Language script functions

	void CurrentLanguageId(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) == 0 ) {
			Return(iArgs, Game::GetInstance().getLanguage());
		}
		else if (ArgCount(iArgs) == 1 ) {
			ArgVarString(language, iArgs, 0);
			Game::GetInstance().setLanguage(language);
		}
		else {
			ArgReportError(iArgs, "Function need 1 or 0 args telling the string language id");
		}
	}
	SCRIPT_FUNCTION(currentLanguage, CurrentLanguageId, "gets or sets the current language");

	// ----------------------------------------------------------------------
	// Fullscreen script functions
	void SetFullscreenScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "Function needs 1 boolean, true fullscreen, false windowed");
		}
		ArgVarBool(fullscreen, iArgs, 0);
		Game::GetInstance().setFullscreen(fullscreen);
	}
	SCRIPT_FUNCTION(setFullscreen, SetFullscreenScriptFunction, "sets fullscreen or not");

	void GetFullscreenScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 0 ) {
			ArgReportError(iArgs, "Function doesnt need any args");
		}
		int value = Game::GetInstance().isFullscreen() ? 1 : 0;
		Return(iArgs, value);
	}
	SCRIPT_FUNCTION(getFullscreen, GetFullscreenScriptFunction, "returns true if fullscreen, false otherwise");

	// ----------------------------------------------------------------------
	// Resolution script functions
	void GetResolutionNameScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 0 ) {
			ArgReportError(iArgs, "Function doesnt need any args");
		}
		const Resolution& res = Game::GetInstance().getResolution();
		std::ostringstream str;
		str << res.width << "x" << res.height;
		Return(iArgs, str.str());
	}
	SCRIPT_FUNCTION(getResolutionName, GetResolutionNameScriptFunction, "Gets the resolution as a string");

}

void SGL_main(const std::string& arg0) {
	lunarlady::Game game(arg0);
	game.loop();
	lunarlady::Registrator().clear();
}