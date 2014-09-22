#ifndef LL_WORLD_STATE_HPP
#define LL_WORLD_STATE_HPP

#include <memory>
#include <string>
#include "lunarlady/State.hpp"
#include "lunarlady/World3.hpp"

namespace lunarlady {
	class WorldState : public State {
	public:
		WorldState(const std::string& iName, const unsigned int iPriority);

		void handleUpdate(real iTime);
		void handleRender(real iTime);
		void handleMouseMovement(const math::vec2& iMovement);
		void handleKey(const sgl::Key& iKey, bool iDown);

		void load(const std::string& iName);
		void unload();
	private:
		std::auto_ptr<World3> mWorld;
	};
}

#endif