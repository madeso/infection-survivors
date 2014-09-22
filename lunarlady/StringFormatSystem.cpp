#include "lunarlady/Game.hpp"
#include "lunarlady/System.hpp"
#include "lunarlady/StringFormat.hpp"

namespace lunarlady {
	class StringFormatSystem : public System {
	public:
		StringFormatSystem() : System("Font format"), mFormat( Registrator().getGeneralStringPath() ) {
		}
		~StringFormatSystem() {
		}

		void step(real iTime) {
		}

		StringFormat mFormat;
	};
	LL_SYSTEM(StringFormatSystem, 400);
}