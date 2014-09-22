#include "lunarlady/Printer.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/StringFormat.hpp"

namespace lunarlady {
	Printer::Printer(Font& iFont, const std::string& iPrintJobName, const math::vec2 iLocation, Justification iJustification, real* iWidth) : mFont(iFont), mPrintJob( StringFormat::GetInstance().getString(iPrintJobName)), mLocation(iLocation), mJustification(iJustification), mSize( Registrator().getFontSize() ), mWidth(iWidth), mCashedWidth(-1), mPrint(true) {
	}
	real Printer::GetHeight(Font& iFont) {
		return iFont->getHeight() * Registrator().getFontSize();
	}
	real Printer::GetBottom(Font& iFont) {
		return iFont->getBottom() * Registrator().getFontSize();
	}
	Printer& Printer::setSize(real iSize) {
		mSize = iSize;
		mCashedWidth = -1;
		return *this;
	}
	Printer::~Printer() {
		if( mPrint ) {
			real* cashedWidth = 0;
			if( mCashedWidth > 0 ) {
				cashedWidth = &mCashedWidth;
			}
			mPrintJob.print(mFont, mSize, mLocation, mJustification, mWidth, cashedWidth, mArguments);
		}
	}

	real Printer::getWidth() {
		mCashedWidth = mPrintJob.getWidth(mFont, mSize, mArguments);
		return mCashedWidth;
	}

	Printer& Printer::dontPrint() {
		mPrint = false;
		return *this;
	}
}