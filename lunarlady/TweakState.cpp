#include "lunarlady/Tweak.hpp"

#include "lunarlady/State.hpp"
#include "lunarlady/World2.hpp"
#include "sgl/sgl_Assert.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Font.hpp"
#include "lunarlady/Object2.hpp"
#include "lunarlady/Printer.hpp"
#include "lunarlady/Language.hpp"

#include <map>
#include <vector>

namespace lunarlady {
	class TweakGraphics;
	namespace {
		TweakGraphics* gTweaks = 0;
		TweakGraphics& GetTweakStateSafe() {
			Assert(gTweaks, "Tweak gfx not initialized");
			return *gTweaks;
		}
	}

	class ImguiObject : public Object2 {
	public:
		typedef long ID;
		ImguiObject(Font& iFont) : mState(S_RENDER), mFont(iFont), mCursorLocation(0.5, 0.5), mCursorDown(false), mHot(0), mActive(0), mDropId(0), mDropIndex(0), mAllowDropReset(true) {
		}

		bool isUpdating() const {
			return mState == S_UPDATE;
		}

		void doRender(real iTime) {
			mState = S_RENDER;
			doGUI(0);
			renderCursor();
		}
		void doUpdate(real iTime) {
			prepare();
			mState = S_UPDATE;
			doGUI(iTime);
			finish();
		}
		virtual void doGUI(real iTime) = 0;

		void frame(ID iID, math::vec2* ioLocation, const std::string& iText, real iWidth, real iHeight, math::vec2* ioBodyLocation) {
			Assert(iID > 0, "The id must be larger than 0");
			const real alpha = 0.95;
			const Rgba bkgColor(102/255.0,103/255.0,89/255.0, alpha);
			const real spacing = 0.01;

			*ioBodyLocation = math::vec2(ioLocation->getX() + spacing, ioLocation->getY() - spacing*2 - Printer::GetBottom(mFont));

			if( isUpdating() ) {
				if( mActive == iID ) {
					*ioLocation += mCursorMovement;
				}
			}

			if( mState == S_RENDER ) {
				const bool hot = mHot == iID;
				const bool active = hot && mActive == iID;
				{
					Printer print(mFont, active ? "imgui.button.active" : "imgui.button", *ioLocation, JUSTIFY_LEFT, 0);
					const std::wstring text(iText.begin(), iText.end());
					print.arg("text", text);
					const real width = math::Max(print.getWidth() + spacing*2, iWidth + 4*spacing);
					const real height = iHeight + spacing * 4 + Printer::GetBottom(mFont);
					quad(bkgColor, ioLocation->getX(), ioLocation->getX() + width, ioLocation->getY() + Printer::GetHeight(mFont), ioLocation->getY() - height);
				}
			}

			if( mState == S_UPDATE ) {
				const real top = ioLocation->getY() + Printer::GetHeight(mFont);
				const real bottom = ioLocation->getY() - Printer::GetBottom(mFont);
				const real left = ioLocation->getX();
				const std::wstring text(iText.begin(), iText.end());
				const real right = left + Printer(mFont, "imgui.button", *ioLocation, JUSTIFY_LEFT, 0).arg("text", text).dontPrint().getWidth();

				if( regionHit(left, right, bottom, top) ) {
					mHot = iID;
					if( mActive == 0 && mCursorDown ) {
						mActive = iID;
					}

					if( !mCursorDown && mHot == iID && mActive == iID ) {
						resetDrop();
					}
					else {
					}
				}
			}
		}

		void resetDrop() {
			if( mAllowDropReset ) {
				mDropId = 0;
				mDropIndex = 0;
			}
		}

		bool dropUp(ID iID, const std::wstring& iText, const math::vec2& iLocation, const std::size_t iMax, const std::vector<std::string>& iValues, std::size_t* oIndex) {
			mAllowDropReset = false;
			bool clicked = button(iID, iText, iLocation);
			mAllowDropReset = true;
			if( clicked ) {
				if( mDropId == 0 ) {
					mDropId = iID;
					mDropIndex = 0;
				}
				else {
					resetDrop();
				}
			}
			bool showing = mDropId == iID;

			const real iDirection = 1; // move to a argument

			bool assignedIndex = false;

			if( showing ) {
				const real spacing = 0.01;
				const real height = Printer::GetHeight(mFont) + spacing;
				const real step = iDirection*height;
				real y = iLocation.getY() + step;
				const real x = iLocation.getX();
				const std::size_t size = iValues.size();
				std::size_t count = 0;
				std::size_t maxToDisplay = iMax;
				bool showPrevious = mDropIndex != 0;
				ID base = iID + 1;
				std::size_t newDisplayIndex = mDropIndex;
				if( showPrevious ) {
					--maxToDisplay;
					mAllowDropReset = false;
					if( button(base, String("imgui.drop.previous"), math::vec2(x, y)) ) {
						--newDisplayIndex;
						if( newDisplayIndex == 1 ) newDisplayIndex = 0;
					}
					mAllowDropReset = true;
					y += step;
					++base;
				}
				bool showNext = mDropIndex + maxToDisplay-1 < size;
				if( showNext ) {
					--maxToDisplay;
				}
				for(std::size_t index=mDropIndex; index<size; ++index) {
					++count;
					if( count > maxToDisplay) {
						break;
					}
					else {
						std::wstring wstring(iValues[index].begin(), iValues[index].end());
						if( button(base + count, wstring, math::vec2(x, y)) ){
							*oIndex = index;
							assignedIndex = true;
						}
						y += step;
					}
				}

				if( showNext ) {
					mAllowDropReset = false;
					if( button(base, String("imgui.drop.next"), math::vec2(x, y)) ) {
						if( newDisplayIndex == 0 ) {
							newDisplayIndex += 2;
						}
						else {
							++newDisplayIndex;
						}
					}
					mAllowDropReset = true;
					y += step;
				}

				mDropIndex = newDisplayIndex;
			}

			return assignedIndex;
		}

		bool button(ID iID, const std::wstring& iText, const math::vec2& iLocation) {
			Assert(iID > 0, "The id must be larger than 0");
			const real alpha = 0.95;
			const Rgba hoverColor(63/255.0,63/255.0,51/255.0,alpha);
			//const Rgba bkgColor(102/255.0,103/255.0,89/255.0, alpha);
			const real spacing = 0.01;

			if( mState == S_RENDER ) {
				const bool hot = mHot == iID;
				const bool active = hot && mActive == iID;
				{
					Printer print(mFont, active ? "imgui.button.active" : "imgui.button", iLocation, JUSTIFY_LEFT, 0);
					print.arg("text", iText);
					const real width = print.getWidth();
					if( hot ) {
						quad(hoverColor, iLocation.getX(), iLocation.getX() + spacing + width, iLocation.getY() + Printer::GetHeight(mFont), iLocation.getY() - Printer::GetBottom(mFont));
					}
				}
				return false;
			}

			if( mState == S_UPDATE ) {
				const real top = iLocation.getY() + Printer::GetHeight(mFont);
				const real bottom = iLocation.getY() - Printer::GetBottom(mFont);
				const real left = iLocation.getX();
				const real right = left + Printer(mFont, "imgui.button", iLocation, JUSTIFY_LEFT, 0).arg("text", iText).dontPrint().getWidth();

				if( regionHit(left, right, bottom, top) ) {
					mHot = iID;
					if( mActive == 0 && mCursorDown ) {
						mActive = iID;
					}

					if( !mCursorDown && mHot == iID && mActive == iID ) {
						resetDrop();
						return true;
					}
					else {
						return false;
					}
				}
				else {
					return false;
				}
			}

			throw "Should not get here";
			return false;
		}

		void checkbox(ID iID, const math::vec2& iLocation, bool* ioState, real iSize) {
			Assert(iID > 0, "The id must be larger than 0");
			const real alpha = 0.95;
			const Rgba hotBkgColor(63/255.0,63/255.0,51/255.0,alpha);
			const Rgba onColor(1.0,1.0,1.0, alpha);
			const Rgba offColor(0.4,0.4,0.4, alpha);
			const real spacing = 0.01;
			const real step = spacing;

			if( mState == S_RENDER ) {
				const bool hot = mHot == iID;
				const real size = iSize - spacing;
				if( hot ) {
					quad(hotBkgColor, iLocation.getX(), iLocation.getX() + iSize, iLocation.getY(), iLocation.getY() - iSize);
				}

				quad(*ioState?onColor:offColor, iLocation.getX()+step, iLocation.getX() + iSize-step, iLocation.getY()-step, iLocation.getY() - iSize+step);
			}

			if( mState == S_UPDATE ) {
				const real top = iLocation.getY();
				const real bottom = iLocation.getY() - iSize;
				const real left = iLocation.getX();
				const real right = left + iSize;

				if( regionHit(left, right, bottom, top) ) {
					mHot = iID;
					if( mActive == 0 && mCursorDown ) {
						mActive = iID;
					}

					if( !mCursorDown && mHot == iID && mActive == iID ) {
						resetDrop();
						*ioState = !(*ioState);
					}
				}
			}
		}

		void slider(ID iID, const math::vec2& iLocation, real* ioValue, real iMin, real iMax, real iWidth, real iHeight) {
			Assert(iID > 0, "The id must be larger than 0");
			const real alpha = 0.95;
			const Rgba hotBkgColor(63/255.0,63/255.0,51/255.0,alpha);
			const Rgba onColor(1.0,1.0,1.0, alpha);
			const Rgba offColor(0.4,0.4,0.4, alpha);
			const real spacing = 0.01;
			const real step = spacing;

			const real base = iMax-iMin;
			const real value = (*ioValue- iMin) / base;

			if( mState == S_RENDER ) {
				const bool hot = mHot == iID;
				if( hot ) {
					quad(hotBkgColor, iLocation.getX(), iLocation.getX() + iWidth, iLocation.getY(), iLocation.getY() - iHeight);
				}

				quad(offColor, iLocation.getX()+step, iLocation.getX() + iWidth-step, iLocation.getY()-step, iLocation.getY() - iHeight+step);
				quad(onColor, iLocation.getX()+step, iLocation.getX() + step + (iWidth - 2*step)* value, iLocation.getY()-step, iLocation.getY() - iHeight+step);
			}

			if( mState == S_UPDATE ) {
				const real top = iLocation.getY();
				const real bottom = iLocation.getY() - iHeight;
				const real left = iLocation.getX();
				const real right = left + iWidth;

				if( regionHit(left, right, bottom, top) ) {
					mHot = iID;
					if( mActive == 0 && mCursorDown ) {
						mActive = iID;
					}

					if( mCursorDown && mHot == iID && mActive == iID ) {
						resetDrop();
						const real a = mCursorLocation.getX() - iLocation.getX() - step;
						const real value = math::limitRange(0, (mCursorLocation.getX() - iLocation.getX() - step) / (iWidth-step*2), 1);
						*ioValue = value*base + iMin;
					}
				}
			}
		}

		void prepare() {
			mHot = 0;
		}
		void finish() {
			mCursorMovement = math::vec2(0,0);
			if( !mCursorDown ) {
				mActive = 0;
			}
			else {
				if( mActive == 0 ) {
					mActive = -1;
				}
			}
		}

		bool regionHit(real left, real right, real bottom, real top) {
			const real x = mCursorLocation.getX();
			const real y = mCursorLocation.getY();

			return ( x > left && x < right && y > bottom && y < top );
		}

		void renderCursor() {
			const real alpha = 0.80;
			const Rgba color(0.0,0.0,1.0,alpha);
			const real spacing = 0.01;
			quad(color, mCursorLocation.getX(), mCursorLocation.getX() + spacing, mCursorLocation.getY(), mCursorLocation.getY() - spacing);
		}

		void onMouseMovement(const math::vec2& iMovement) {
			mCursorLocation += iMovement;
			mCursorMovement += iMovement;
		}
		void onMouseButton(bool iDown) {
			mCursorDown = iDown;
		}
	private:
		enum {
			S_RENDER,
			S_UPDATE
		} mState;
		Font& mFont;

		bool mCursorDown;
		ID mHot;
		ID mActive;
		ID mDropId;
		std::size_t mDropIndex;
		bool mAllowDropReset;

		math::vec2 mCursorLocation;
		math::vec2 mCursorMovement;
	};

	
	class Tweakable {
	public:
		Tweakable() : mWidth(0.1), mHeight(0.1), mEnabled(false), mID(sID), mLocation(0.5, 0.5) {
			sID += 2;
		}
		void gui(ImguiObject* iObject) {
			if( mEnabled ) {
				math::vec2 contentLocation;
				iObject->frame(mID, &mLocation, mName, mWidth, mHeight, &contentLocation);
				doGui(mID+1, contentLocation, iObject);
			}
		}
		bool isDisabled() const {
			return !mEnabled;
		}
		virtual void doGui(ImguiObject::ID iID, const math::vec2& iBase, ImguiObject* iObject) = 0;
		const std::string& getName() const {
			return mName;
		}
		void setName(const std::string& iName) {
			mName = iName;
		}
		void toggle() {
			mEnabled = !mEnabled;
		}
	protected:
		real mWidth;
		real mHeight;
	private:
		ImguiObject::ID mID;
		bool mEnabled;
		math::vec2 mLocation;
		std::string mName;

		static ImguiObject::ID sID;
	};

	const std::size_t MAX_DROP = 9;

	ImguiObject::ID Tweakable::sID = MAX_DROP + 3;

	template<class Type>
	class TweakData : public Tweakable {
	public:
		TweakData() : value(0), lifeConsumed(0) {
		}
		void update() {
			lifeConsumed += 1;
		}
		void set(const std::string& iName, Type* ioValue) {
			lifeConsumed = 0;
			setName(iName);
			value = ioValue;
		}
		void set(const std::string& iName, Type* ioValue, Type iMin, Type iMax) {
			lifeConsumed = 0;
			setName(iName);
			value = ioValue;
			min = iMin;
			max = iMax;
		}
		bool isDead() {
			return lifeConsumed > 3;
		}

		void doGui(ImguiObject::ID iID, const math::vec2& iBase, ImguiObject* iObject);
	private:
		Type* value;
		Type min;
		Type max;
		int lifeConsumed;
	};

	void TweakData<bool>::doGui(ImguiObject::ID iID, const math::vec2& iBase, ImguiObject* iObject) {
		const real size = 0.05;
		iObject->checkbox(iID, iBase, value, size);
		mWidth = size;
		mHeight = size;
	}

	void TweakData<real>::doGui(ImguiObject::ID iID, const math::vec2& iBase, ImguiObject* iObject) {
		mWidth = 0.5;
		mHeight = 0.05;
		iObject->slider(iID, iBase, value, min, max, mWidth, mHeight);
	}

	template<class Type>
	class Container {
	public:
		void tweak(const std::string& iName, Type* ioValue) {
			mMap[ioValue].set(iName, ioValue);
		}
		void tweak(const std::string& iName, Type* ioValue, Type iMin, Type iMax) {
			mMap[ioValue].set(iName, ioValue, iMin, iMax);
		}
		void update() {
			std::for_each(mMap.begin(), mMap.end(), UpdateData);
			for(Map::iterator index = mMap.begin(); index != mMap.end();) {
				if( IsDataDead(*index) ) {
					index = mMap.erase(index);
				}
				else {
					++index;
				}
			}
		}
		void doGUI(ImguiObject* ioObject) {
			for(Map::iterator index = mMap.begin(); index != mMap.end(); ++index) {
				index->second.gui(ioObject);
			}
		}
		void fill(std::vector<std::string>& oContainer, std::vector<Tweakable*>& oTweakables) {
			for(Map::iterator index = mMap.begin(); index != mMap.end(); ++index) {
				oContainer.push_back( index->second.getName() );
				oTweakables.push_back( &(index->second) );
			}
		}

		typedef Type* Key;
		typedef TweakData<Type> Value;
		typedef std::map<Key, Value > Map;
	private:
		static void UpdateData(typename Map::value_type& iData) {
			iData.second.update();
		}
		static bool IsDataDead(typename Map::value_type& iData) {
			return iData.second.isDead();
		}
		Map mMap;
	};


	class TweakGraphics : public ImguiObject {
	public:
		TweakGraphics(Font& iFont) : ImguiObject(iFont) {
			Assert(!gTweaks, "Tweak gfx already initialized");
			gTweaks = this;
		}
		~TweakGraphics() {
			Assert(gTweaks, "Tweak gfx not initialized");
			gTweaks = 0;
		}
		void doGUI(real iTime) {
			const real spacing = 0.01;
			real y = spacing*2;
			real x = spacing;

			if( isUpdating() ) {
				mBool.update();
				mReal.update();
			}

			mBool.doGUI(this);
			mReal.doGUI(this);

			std::vector<std::string> items;
			std::vector<Tweakable*> tweaks;
			mReal.fill(items, tweaks);
			mBool.fill(items, tweaks);
			

			std::size_t index=0;
			if( dropUp(1, String("tweak.start"), math::vec2(x,y), MAX_DROP, items, &index) ) {
				tweaks[index]->toggle();
			}
		}

		void tweak(const std::string& iName, bool* ioValue) {
			mBool.tweak(iName, ioValue);
		}
		void tweak(const std::string& iName, real* ioValue, real iMin, real iMax) {
			mReal.tweak(iName, ioValue, iMin, iMax);
		}
	private:
		Container<bool> mBool;
		Container<real> mReal;
	};

	class TweakState : public State {
	public:
		TweakState() : State("tweakers", 500, false, true, false), mFont(FontDescriptor(mWorld, "fonts/big.fnt")) {
			mWorld.add( new TweakGraphics(mFont) );
		}
		~TweakState() {
		}

		void doFrame(real iTime) {
		}
		void doTick(real iTime) {
			mWorld.update(iTime);
		}
		void doRender(real iTime) {
			mWorld.render(iTime);
		}
		void onMouseMovement(const math::vec2& iMovement) {
			GetTweakStateSafe().onMouseMovement(iMovement);
		}
		void onKey(const sgl::Key& iKey, bool iDown) {
			if( iKey == sgl::Key::Mouse_Left ) {
				GetTweakStateSafe().onMouseButton(iDown);
			}
		}
	private:
		World2 mWorld;
		Font mFont;
	};

	LL_STATE(TweakState)

	void TweakSingleImpl(const std::string& iName, bool* ioValue) {
		GetTweakStateSafe().tweak(iName, ioValue);
	}
	void TweakSliderImpl(const std::string& iName, real* ioValue, real iMin, real iMax) {
		GetTweakStateSafe().tweak(iName, ioValue, iMin, iMax);
	}
}