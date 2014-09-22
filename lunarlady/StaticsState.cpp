#include "lunarlady/Display.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/State.hpp"
#include "lunarlady/World2.hpp"
#include "lunarlady/ComponentObject2.hpp"
#include "lunarlady/Font.hpp"
#include "lunarlady/Printer.hpp"

namespace lunarlady {
	struct DisplayInfo {
		void set(const std::wstring& iValue, bool iOk) {
			value = iValue;
			ok = iOk;
			life = 0;
		}
		void set(const std::wstring& iValue, const math::vec2& iLocation, bool iOk) {
			value = iValue;
			ok = iOk;
			location = iLocation;
			life = 0;
		}
		void step(const real iTime) {
			life += iTime;
		}
		bool removeMe() {
			return life > 0.3;
		}
		std::wstring value;
		bool ok;
		real life;
		math::vec2 location;
	};
	typedef boost::shared_ptr<DisplayInfo> DisplayInfoPtr;
	typedef std::map<std::string, DisplayInfoPtr> DisplayInfoMap;

	namespace {
		void Update(DisplayInfoMap& iMap, real iTime) {
			for(DisplayInfoMap::iterator i=iMap.begin(); i != iMap.end(); ) {
				DisplayInfoPtr displayInfo = i->second;
				displayInfo->step(iTime);
				if( displayInfo->removeMe() ) {
					i = iMap.erase(i);
				}
				else {
					++i;
				}
			}
		}
	}

	class DisplayGraphics : public Object2 {
	public:
		DisplayGraphics(Font& iFont) : mFont(iFont){
		}
		void doRender(real iTime) {
			mFont.load(); // since this can be on to of the loader this has to be done
			const real x = Registrator().getAspect() - 0.01;
			real y = 0.01;
			const real step = Printer::GetHeight(mFont) + 0.001;

			for(DisplayInfoMap::iterator i=mDisplay.begin(); i != mDisplay.end(); ++i) {
				DisplayInfoPtr displayInfo = i->second;
				const std::string jobName = (displayInfo->ok) ? "display.ok" : "display.bad";
				const std::wstring name(i->first.begin(), i->first.end());
				Printer(mFont, jobName, math::vec2(x,y), JUSTIFY_RIGHT, 0).arg("name", name).arg("text", displayInfo->value);
				y+=step;
			}
			for(DisplayInfoMap::iterator i=mDisplayLocation.begin(); i != mDisplayLocation.end(); ++i) {
				DisplayInfoPtr displayInfo = i->second;
				const std::string jobName = (displayInfo->ok) ? "display.ok" : "display.bad";
				const std::wstring name(i->first.begin(), i->first.end());
				Printer(mFont, jobName, displayInfo->location, JUSTIFY_LEFT, 0).arg("name", name).arg("text", displayInfo->value);
			}
		}
		void doUpdate(real iTime) {
			Update(mDisplay, iTime);
			Update(mDisplayLocation, iTime);
		}
		void set(const std::string& iName, const std::wstring& iValue, bool iOk) {
			DisplayInfoMap::iterator res = mDisplay.find(iName);
			DisplayInfoPtr display;
			if( res == mDisplay.end() ) {
				display.reset( new DisplayInfo() );
				mDisplay.insert( DisplayInfoMap::value_type(iName, display) );
			}
			else {
				display = res->second;
			}

			display->set(iValue, iOk);
		}
		void set(const std::string& iName, const std::wstring& iValue, const math::vec2 iLocation, bool iOk) {
			DisplayInfoMap::iterator res = mDisplayLocation.find(iName);
			DisplayInfoPtr display;
			if( res == mDisplayLocation.end() ) {
				display.reset( new DisplayInfo() );
				mDisplayLocation.insert( DisplayInfoMap::value_type(iName, display) );
			}
			else {
				display = res->second;
			}

			display->set(iValue, iLocation, iOk);
		}
	private:
		Font& mFont;

		DisplayInfoMap mDisplay;
		DisplayInfoMap mDisplayLocation;
	};

	namespace {
		DisplayGraphics* gGraphics=0;
	}

	void ImpDisplayString(const std::string& iName, const std::wstring& iValue, bool iOk) {
		gGraphics->set(iName, iValue, iOk);
	}
	void ImpDisplayStringAtLocation(const std::string& iName, const std::wstring& iValue, const math::vec2 iLocation, bool iOk) {
		gGraphics->set(iName, iValue, iLocation, iOk);
	}

	class DisplayState : public State {
	public:
		DisplayState() : State("display", 600, true, true, 
#ifdef NDEBUG
			false
#else
			true
#endif
			), mWorld(), mFont(FontDescriptor(mWorld, "fonts/big.fnt")) {
			gGraphics = new DisplayGraphics(mFont);
			mWorld.add( gGraphics );
			mFont.load();
		}

		void doFrame(real iTime) {
			mWorld.update(iTime);
		}
		void doTick(real iTime) {
		}
		void doRender(real iTime) {
			mWorld.render(iTime);
		}

		void onMouseMovement(const math::vec2& iMovement) {
			sendMouseMovement(iMovement);
		}
		void onKey(const sgl::Key& iKey, bool iDown) {
			sendKey(iKey, iDown);
		}

		World2 mWorld;
		Font mFont;
	};

	LL_STATE(DisplayState);
}