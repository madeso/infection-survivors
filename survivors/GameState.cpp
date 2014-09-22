#include "lunarlady/WorldState.hpp"
#include "lunarlady/Game.hpp"

namespace survivors {
	class GameState : public ::lunarlady::WorldState {
	public:
		GameState() : ::lunarlady::WorldState( "game", 200 ) {
		}
		void doFrame(::lunarlady::real iTime) {
		}
		void doTick(::lunarlady::real iTime) {
			handleUpdate(iTime);
		}
		void doRender(::lunarlady::real iTime) {
			handleRender(iTime);
		}

		void onMouseMovement(const ::lunarlady::math::vec2& iMovement) {
			handleMouseMovement(iMovement);
		}
		void onKey(const ::sgl::Key& iKey, bool iDown) {
			if( iKey==sgl::Key::Escape && iDown ) {
				lunarlady::ExitGame();
				return;
			}
			handleKey(iKey, iDown);
		}
	};

	LL_STATE(GameState);
}