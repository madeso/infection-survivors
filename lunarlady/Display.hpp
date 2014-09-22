#ifndef LL_DISPLAY_HPP
#define LL_DISPLAY_HPP

#include <string>
#include "lunarlady/math/vec2.hpp"
#include "boost/lexical_cast.hpp"
#include <sstream>

namespace lunarlady {
	void ImpDisplayString(const std::string& iName, const std::wstring& iValue, bool iOk);
	void ImpDisplayStringAtLocation(const std::string& iName, const std::wstring& iValue, const math::vec2& iLocation, bool iOk);

	template<class T>
	void ImpDisplay(const std::string& iName, const T& iValue, bool iOk) {
		ImpDisplayString(iName, ::boost::lexical_cast<std::wstring>(iValue), iOk);
	}

	template<class T>
	void ImpDisplayAtLocation(const std::string& iName, const T& iValue, const math::vec2& iLocation, bool iOk) {
		ImpDisplayStringAtLocation(iName, ::boost::lexical_cast<std::wstring>(iValue), iLocation, iOk);
	}

	template<class T>
	std::wstring ToWideString(const T& iValue, int iNumbers) {
		std::wstringstream str;
		str.precision(iNumbers);
		str.width(iNumbers);
		str << std::fixed << std::showpoint << iValue;
		return str.str();
	}

	template<class T>
	void ImpDisplayPrecision(const std::string& iName, const T& iValue, int iNumbers, bool iOk) {
		ImpDisplayString(iName, ToWideString(iValue, iNumbers), iOk);
	}

	template<class T>
	void ImpDisplayPrecisionAtLocation(const std::string& iName, const T& iValue, const math::vec2& iLocation, int iNumbers, bool iOk) {
		ImpDisplayStringAtLocation(iName, ToWideString(iValue, iNumbers), iLocation, iOk);
	}

#define DisplayString(iName, iValue, iOk)										ImpDisplayString(iName, iValue, iOk)
#define DisplayStringAtLocation(iName, iValue, iLocation, iOk)					ImpDisplayStringAtLocation(iName, iValue, iLocation, iOk)
#define Display(iName, iValue, iOk)												ImpDisplay(iName, iValue, iOk)
#define DisplayAtLocation(iName, iValue, iLocation, iOk)						ImpDisplayAtLocation(iName, iValue, iLocation, iOk)
#define DisplayPrecision(iName, iValue, iNumbers, iOk)							ImpDisplayPrecision(iName, iValue, iNumbers, iOk)
#define DisplayPrecisionAtLocation(iName, iValue, iLocation, iNumbers, iOk)		ImpDisplayPrecisionAtLocation(iName, iValue, iLocation, iNumbers, iOk)
}

#endif