#include <sstream>
#include "convertColorBetweenGreen.hpp"

namespace lunarlady {
namespace convert {
	const int ColorToGreen(Color::Data d) {
		switch(d) {
			case Color::Black:
				return 0x00;
			case Color::Silver:
				return 0xC0;
			case Color::Maroon:
				return 0x00;
			case Color::Red:
				return 0x00;
			case Color::Navy:
				return 0x00;
			case Color::Blue:
				return 0x00;
			case Color::Purple:
				return 0x00;
			case Color::Fuchia:
				return 0x00;
			case Color::Green:
				return 0x80;
			case Color::Lime:
				return 0xFF;
			case Color::Olive:
				return 0x80;
			case Color::Yellow:
				return 0xFF;
			case Color::Teal:
				return 0x80;
			case Color::Aqua:
				return 0xFF;
			case Color::Gray:
				return 0x80;
			case Color::White:
				return 0xFF;
			case Color::Orange:
				return 0xA5;
			default:
				{
					std::ostringstream message;
					message << Color::asString(d) << " is not a valid Green";
					throw std::runtime_error(message.str());
				}
		}
	}
} // convert

} //lunarlady

