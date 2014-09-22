#include "lunarlady/Menu.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/math/math.hpp"

namespace lunarlady {
	Menu::Menu(const std::string& iName, const int iPriority, bool iUpdatePrevious, bool iRenderPrevious, bool iIsEnabled, const std::string& iMenuFile)
		: State(iName, iPriority, iUpdatePrevious, iRenderPrevious, iIsEnabled), mWorld(), mContainer(mWorld) {
			mContainer.registerDefaultBuilders();
			mContainer.load(iMenuFile);
	}

	void Menu::doFrame(real iTime) {
		mContainer.init();
		mContainer.step();
		mWorld.update(iTime);
	}
	void Menu::doTick(real iTime) {
	}
	void Menu::doRender(real iTime) {
		mContainer.init();
		mWorld.render(iTime);
	}
	void Menu::onMouseMovement(const math::vec2& iMovement) {
		mContainer.sendMouseMovement(iMovement * math::Map01ToMultiplier(GetMenuSensivity(), 3) );
	}
	void Menu::onKey(const sgl::Key& iKey, bool iDown) {
		mContainer.sendKey(iKey, iDown);
	}
	void Menu::onChar(const wchar_t iChar) {
		mContainer.sendChar(iChar);
	}
	void Menu::onFocus(bool iGained) {
		mContainer.onFocus(iGained);
	}
}