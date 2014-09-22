#ifndef LL_MENU_HPP
#define LL_MENU_HPP

#include "lunarlady/State.hpp"
#include "lunarlady/World2.hpp"
#include "lunarlady/ComponentObject2.hpp"

namespace lunarlady {
	class Menu : public State {
	public:
		Menu(const std::string& iName, const int iPriority, bool iUpdatePrevious, bool iRenderPrevious, bool iIsEnabled, const std::string& iMenuFile);

		void doFrame(real iTime);
		void doTick(real iTime);
		void doRender(real iTime);

		void onMouseMovement(const math::vec2& iMovement);
		void onKey(const sgl::Key& iKey, bool iDown);
		void onChar(const wchar_t iChar);

	protected:
		virtual void onFocus(bool iGained);
	private:
		World2 mWorld;
		ComponentObjectContainer2 mContainer;
	};
}

#endif