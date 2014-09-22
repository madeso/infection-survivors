#ifndef LL_TEXT_OBJECT2_HPP
#define LL_TEXT_OBJECT2_HPP

#include <string>

#include "lunarlady/Object2.hpp"
#include "lunarlady/math/vec2.hpp"
#include "lunarlady/Font.hpp"

namespace lunarlady {
	class TextObject2 : public Object2 {
	public:
		TextObject2(const math::vec2 iLocation, Font& iFont, const std::string& iText);
		~TextObject2();

		virtual void render(real iTime);
		virtual void update(real iTime);
	private:
		Font& mFont;
		math::vec2 mLocation;
		std::string mText;
		real mTime;
	};
}

#endif