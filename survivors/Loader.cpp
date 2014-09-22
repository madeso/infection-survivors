#include "lunarlady/Game.hpp"
#include "lunarlady/Loader.hpp"

namespace infection {
	class Loader : public ::lunarlady::Loader {
	public:
		Loader() : ::lunarlady::Loader("menu/loader.xml") {
		}
	};

	LL_STATE(Loader);
}