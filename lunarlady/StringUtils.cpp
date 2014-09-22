#include "lunarlady/StringUtils.hpp"

#include <cassert>
#include "boost/tokenizer.hpp"
#include "boost/smart_ptr.hpp"

namespace lunarlady {
	void SplitString(const std::string& iDelimiterString, const std::string& iStringToSplit, std::vector<std::string>* iResult) {
		assert(iResult);
		boost::char_separator<char> sep(iDelimiterString.c_str());
		boost::tokenizer< boost::char_separator<char> > tok(iStringToSplit, sep);
		std::copy(tok.begin(), tok.end(), std::back_inserter(*iResult));
	}

	std::string TrimRight(std::string iStringToTrim,const std::string& iTrimCharacters) {
		return iStringToTrim.erase(iStringToTrim.find_last_not_of(iTrimCharacters)+1);
	}

	std::string TrimLeft(std::string iStringToTrim,const std::string& iTrimCharacters) {
		return iStringToTrim.erase(0,iStringToTrim.find_first_not_of(iTrimCharacters));
	}

	std::string Trim(const std::string& iStringToTrim,const std::string& iTrimCharacters) {
		return TrimRight( TrimLeft(iStringToTrim, iTrimCharacters), iTrimCharacters);
	}

	void SplitString(const std::wstring& iDelimiterString, const std::wstring& iStringToSplit, std::vector<std::wstring>* iResult) {
		assert(iResult);
		boost::char_separator<wchar_t> sep(iDelimiterString.c_str());
		boost::tokenizer<boost::char_separator<wchar_t>, std::wstring::const_iterator, std::wstring> tok(iStringToSplit, sep);
		std::copy(tok.begin(), tok.end(), std::back_inserter(*iResult));
	}

	std::wstring TrimRight(std::wstring iStringToTrim,const std::wstring& iTrimCharacters) {
		return iStringToTrim.erase(iStringToTrim.find_last_not_of(iTrimCharacters)+1);
	}

	std::wstring TrimLeft(std::wstring iStringToTrim,const std::wstring& iTrimCharacters) {
		return iStringToTrim.erase(0,iStringToTrim.find_first_not_of(iTrimCharacters));
	}

	std::wstring Trim(const std::wstring& iStringToTrim,const std::wstring& iTrimCharacters) {
		return TrimRight( TrimLeft(iStringToTrim, iTrimCharacters), iTrimCharacters);
	}
	std::wstring ToLower(const std::wstring& iString) {
		std::wstring result = iString;
		//std::for_each(result.begin(), result.end(), tolower);
		std::transform(result.begin(), result.end(), result.begin(), tolower);
		return result;
	}

	bool StartsWith(const std::string iStringToTest, const std::string& iStart) {
		const std::string::size_type length = iStart.length();
		const std::string::size_type otherLength = iStringToTest.length();
		if( otherLength < length ) return false;
		const std::string start = iStringToTest.substr(0, length);
		return start == iStart;
	}
	bool EndsWith(const std::string iStringToTest, const std::string& iEnd) {
		const std::string::size_type length = iEnd.length();
		const std::string::size_type otherLength = iStringToTest.length();
		if( otherLength < length ) return false;
		const std::string end = iStringToTest.substr(otherLength-length, length);
		return end == iEnd;
	}

	std::string ToLower(const std::string& iString) {
		std::string result = iString;
		//std::for_each(result.begin(), result.end(), tolower);
		std::transform(result.begin(), result.end(), result.begin(), tolower);
		return result;
	}

	void StringReplace(std::string* ioString, const std::string& iToFind, const std::string& iToReplace) {
		std::size_t index = ioString->find(iToFind);
		const std::size_t findLength = iToFind.length();
		while (index!=std::string::npos) {
			ioString->erase(index, findLength);
			ioString->insert(index, iToReplace);
			index = ioString->find(iToFind, index);
		}
	}
	void StringReplace(std::wstring* ioString, const std::wstring& iToFind, const std::wstring& iToReplace) {
		std::size_t index = ioString->find(iToFind);
		const std::size_t findLength = iToFind.length();
		while (index!=std::wstring::npos) {
			ioString->erase(index, findLength);
			ioString->insert(index, iToReplace);
			index = ioString->find(iToFind, index);
		}
	}
}