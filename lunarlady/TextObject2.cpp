#include "lunarlady/TextObject2.hpp"
#include "lunarlady/Printer.hpp"
namespace lunarlady {
	TextObject2::TextObject2(const math::vec2 iLocation, Font& iFont, const std::string& iText) : mFont(iFont), mLocation(iLocation), mText(iText), mTime(0) {
	}
	TextObject2::~TextObject2() {
	}

	void TextObject2::render(real iTime) {
		//LL_PRINT(mFont, 0.50, mLocation, L"Heart" << FontColor(Color::Red) << FontSize( 1 + 0.5*math::sin(mTime) ) << L"Beat" << FontSize(1) << FontColor(Color::White) << L"!" );
		Printer(mFont, mText, mLocation, JUSTIFY_LEFT, 0);
	}
	void TextObject2::update(real iTime) {
		mTime += iTime * 0.5;
		if( mTime < math::PI*2 )
			mTime -= math::PI*2;
	}
}