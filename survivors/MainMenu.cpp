#include "lunarlady/Menu.hpp"
#include "lunarlady/Game.hpp"

namespace infection {
	class MainMenu : public ::lunarlady::Menu {
	public:
		MainMenu() : Menu("main", 100, false, false, true, "menu/main.xml") {
		}
	};

	LL_STATE(MainMenu);
}