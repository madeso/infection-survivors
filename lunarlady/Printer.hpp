#ifndef LL_PRINTER_HPP
#define LL_PRINTER_HPP

#include "boost/lexical_cast.hpp"
#include "boost/utility.hpp"

#include "lunarlady/Font.hpp"
#include "lunarlady/math/vec2.hpp"
#include "lunarlady/PrintJob.hpp"

namespace lunarlady {
	class Printer : boost::noncopyable {
	public:
		Printer(Font& iFont, const std::string& iPrintJobName, const math::vec2 iLocation, Justification iJustification, real* iWidth);
		Printer& setSize(real iSize);
		~Printer();
		
		template<class T>
		Printer& arg(const std::string& iName, const T& iValue) {
			mArguments.insert( Argument(iName, boost::lexical_cast<std::wstring>(iValue)) );
			mCashedWidth = -1;
			return *this;
		}

		template<class T>
		Printer& arg(const std::string& iName, const T& iValue, int iNumbers) {
			std::wstringstream str;
			str.precision(iNumbers);
			str.width(iNumbers);
			str << std::fixed << std::showpoint << iValue;
			mArguments.insert( Argument(iName, str.str()) );
			mCashedWidth = -1;
			return *this;
		}

		static real GetHeight(Font& iFont);
		static real GetBottom(Font& iFont);

		real getWidth();
		Printer& dontPrint();
	private:
		Font& mFont;
		const PrintJob mPrintJob;
		const math::vec2 mLocation;
		const Justification mJustification;
		real mSize;
		real* mWidth;
		real mCashedWidth;
		bool mPrint;

		ArgumentMap mArguments;
	};
}

#endif