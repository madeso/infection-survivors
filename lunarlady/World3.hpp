#ifndef LL_WORLD3_HPP
#define LL_WORLD3_HPP

#include <string>
#include <memory>
#include "sgl/Key.hpp"
#include "lunarlady/Types.hpp"

namespace lunarlady {
	class ReadFile;
	namespace math {
		class vec2;
	}
	class World3 {
	public:
		virtual ~World3() {}

		static World3* Load(const std::string& iName);

		virtual void update(real iTime) = 0;
		virtual void render(real iTime) = 0;
		virtual void onMouseMovement(const math::vec2& iMovement) = 0;
		virtual void onKey(const sgl::Key& iKey, bool iDown) = 0;
	};
}

#endif