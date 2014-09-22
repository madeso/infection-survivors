#ifndef LL_STATE_HPP
#define LL_STATE_HPP

#include <string>

#include "boost/smart_ptr.hpp"
#include "boost/utility.hpp"

#include "lunarlady/Types.hpp"
#include "lunarlady/math/vec2.hpp"

#include "sgl/Key.hpp"

namespace lunarlady {
	class State : boost::noncopyable {
	public:
		State(const std::string& iName, const int iPriority, bool iUpdatePrevious, bool iRenderPrevious, bool iIsEnabled);
		virtual ~State();

		virtual void doFrame(real iTime) = 0;
		virtual void doTick(real iTime) = 0;
		virtual void doRender(real iTime) = 0;
		virtual void onMouseMovement(const math::vec2& iMovement) = 0;
		virtual void onKey(const sgl::Key& iKey, bool iDown) = 0;

		virtual void onChar(const wchar_t iChar);

		void frame(real iTime);
		void tick(real iTime);
		void render(real iTime);

		const std::string& getName() const;
		const int getPriority() const;

		bool isEnabled() const;
		void setEnabled(bool iIsEnabled);

		void setNextState(State* iNext);
		void setPreviousState(State* iPrevious);
		void clearStateLinks();

		void enable();
		void disable();

		virtual void onEnabled() {}
		virtual void onDisabled() {}

		void focus(bool iGained);
	protected:
		virtual void onFocus(bool iGained) {}

		void sendMouseMovement(const math::vec2& iMovement);
		void sendKey(const sgl::Key& iKey, bool iDown);
		void sendChar(const wchar_t iChar);
	private:
		const std::string mName;
		const int mPriority;

		const bool mUpdatePrevious;
		const bool mRenderPrevious;
		bool mFocus;
		
		bool mIsEnabled;
		State* mPreviousState;
		State* mNextState;
	};

	typedef boost::shared_ptr<State> StatePtr;
}

#endif