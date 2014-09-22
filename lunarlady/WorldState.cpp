#include "lunarlady/WorldState.hpp"
#include "lunarlady/World3.hpp"
#include "lunarlady/Script.hpp"
#include "lunarlady/Tweak.hpp"

namespace lunarlady {
	class LoadWorldStateFunction {
	public:
		LoadWorldStateFunction(WorldState* iState) : mState(iState) {
		}
		void operator()(FunctionArgs& iArgs) {
			if( ArgCount(iArgs) != 1 ) {
				ArgReportError(iArgs, "function need 1 arg: the level name");
			}
			ArgVarString(name, iArgs, 0);
			mState->load(name);
			mState->enable();
		}
	private:
		WorldState* mState;
	};
	class UnloadWorldStateFunction {
	public:
		UnloadWorldStateFunction(WorldState* iState) : mState(iState) {
		}
		void operator()(FunctionArgs& iArgs) {
			if( ArgCount(iArgs) != 0 ) {
				ArgReportError(iArgs, "function doesnt need any args");
			}
			mState->unload();
			mState->disable();
		}
	private:
		WorldState* mState;
	};

	WorldState::WorldState(const std::string& iName, const unsigned int iPriority) : State(iName, iPriority, false, false, false) {
		RegisterFunction(iName + std::string(".load"), LoadWorldStateFunction(this), std::string("loads a world for ") + iName);
		RegisterFunction(iName + std::string(".unload"), UnloadWorldStateFunction(this), std::string("unloads a world for ") + iName);
	}

	void WorldState::handleUpdate(real iTime) {
		if( mWorld.get() ) {
			mWorld->update(iTime);
		}
	}

	void WorldState::handleRender(real iTime) {
		static bool render = true;
		TweakSingle("renderWorld", &render);

		if( render ) {
			if( mWorld.get() ) {
				mWorld->render(iTime);
			}
		}
	}

	void WorldState::handleMouseMovement(const math::vec2& iMovement) {
		if( mWorld.get() ) {
			mWorld->onMouseMovement(iMovement);
		}
	}

	void WorldState::handleKey(const sgl::Key& iKey, bool iDown) {
		if( mWorld.get() ) {
			mWorld->onKey(iKey, iDown);
		}
	}

	void WorldState::load(const std::string& iName) {
		mWorld.reset( World3::Load(iName) );
	}
	void WorldState::unload() {
		mWorld.reset( );
	}
}