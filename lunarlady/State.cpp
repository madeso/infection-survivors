#include "lunarlady/State.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Log.hpp"
#include "lunarlady/Script.hpp"
#include "sgl/sgl_Assert.hpp"

namespace lunarlady {

	class DisableStateFunction {
	public:
		DisableStateFunction(State* iState) : mState(iState) {
		}
		void operator()(FunctionArgs& iArgs) {
			mState->disable();
		}
	private:
		State* mState;
	};

	State::State(const std::string& iName, const int iPriority, bool iUpdatePrevious, bool iRenderPrevious, bool iIsEnabled)
		:  mName(iName), mPriority(iPriority), mUpdatePrevious(iUpdatePrevious), mRenderPrevious(iRenderPrevious), mIsEnabled(iIsEnabled), mPreviousState(0), mNextState(0), mFocus(false){
			LOG1( "Initializing " << mName << " state");
			RegisterFunction(iName + std::string(".disable"), DisableStateFunction(this), std::string("disables ") + iName);
	}
	State::~State() {
		LOG1( "de-Initializing " << mName << " state");
	}

	const std::string& State::getName() const {
		return mName;
	}
	const int State::getPriority() const {
		return mPriority;
	}

	void State::onChar(const wchar_t iChar) {
		sendChar(iChar);
	}

	void State::frame(real iTime) {
		doFrame(iTime);
		if( mUpdatePrevious && mPreviousState ) {
			mPreviousState->frame(iTime);
		}
	}

	void State::tick(real iTime) {
		doTick(iTime);
		if( mUpdatePrevious && mPreviousState ) {
			mPreviousState->tick(iTime);
		}
	}

	void State::render(real iTime) {
		if( mRenderPrevious && mPreviousState ) {
			mPreviousState->render(iTime);
		}
		doRender(iTime);
	}

	void State::sendMouseMovement(const math::vec2& iMovement) {
		if( mPreviousState ) {
			mPreviousState->onMouseMovement(iMovement);
		}
	}
	void State::sendKey(const sgl::Key& iKey, bool iDown) {
		if( mPreviousState ) {
			mPreviousState->onKey(iKey, iDown);
		}
	}
	void State::sendChar(const wchar_t iChar) {
		if( mPreviousState ) {
			mPreviousState->onChar(iChar);
		}
	}

	bool State::isEnabled() const {
		return mIsEnabled;
	}

	void State::setNextState(State* iNext) {
		mNextState = iNext;
	}

	void State::setPreviousState(State* iPrevious) {
		mPreviousState = iPrevious;
	}

	void State::clearStateLinks() {
		mNextState = 0;
		mPreviousState = 0;
	}

	void State::enable() {
		setEnabled(true);
		onEnabled();
	}

	void State::disable() {
		setEnabled(false);
		onDisabled();
	}

	void State::setEnabled(bool iIsEnabled) {
		mIsEnabled = iIsEnabled;
		UpdateStateLinks();
	}

	void State::focus(bool iFocus) {
		if( mFocus != iFocus ) {
			mFocus = iFocus;
			onFocus(iFocus);
		}
		if( mPreviousState ) {
			bool gained = iFocus;
			if( !mUpdatePrevious ) {
				gained = false;
			}
			mPreviousState->focus(gained);
		}
	}
}