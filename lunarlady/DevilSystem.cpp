#include "IL/il.h"

#include "lunarlady/System.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Log.hpp"
#include "lunarlady/Error.hpp"

namespace lunarlady {
	class DevilSystem : public System {
	public:
		DevilSystem() : System("DevIL") {
			if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
				//DisplayError("DevIL version is different...exiting!");
				//return false;
				LOG1("DevIL version is different...exiting!");
				throw DevILError("DevIL version is different");
			}
			ilInit();
			ilEnable(IL_ORIGIN_SET);
			ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		}
		~DevilSystem() {
		}

		void step(real iTime) {
		}
	};
	LL_SYSTEM(DevilSystem, 400);
}