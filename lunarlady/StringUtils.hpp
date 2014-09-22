#ifndef LL_STRING_UTILS_HPP
#define LL_STRING_UTILS_HPP

#include <vector>
#include <string>

namespace lunarlady {
	const std::string SPACE_CHARACTERS = " \n\t";
	const std::wstring SPACE_CHARACTERSW = L" \n\t";
	void SplitString(const std::string& iDelimiterString, const std::string& iStringToSplit, std::vector<std::string>* iResult);
	std::string TrimRight(std::string iStringToTrim,const std::string& iTrimCharacters = SPACE_CHARACTERS );
	std::string TrimLeft(std::string iStringToTrim,const std::string& iTrimCharacters = SPACE_CHARACTERS );
	std::string Trim(const std::string& iStringToTrim,const std::string& iTrimCharacters = SPACE_CHARACTERS );

	void SplitString(const std::wstring& iDelimiterString, const std::wstring& iStringToSplit, std::vector<std::wstring>* iResult);
	std::wstring TrimRight(std::wstring iStringToTrim,const std::wstring& iTrimCharacters = SPACE_CHARACTERSW );
	std::wstring TrimLeft(std::wstring iStringToTrim,const std::wstring& iTrimCharacters = SPACE_CHARACTERSW );
	std::wstring Trim(const std::wstring& iStringToTrim,const std::wstring& iTrimCharacters = SPACE_CHARACTERSW );

	bool StartsWith(const std::string iStringToTest, const std::string& iStart);
	bool EndsWith(const std::string iStringToTest, const std::string& iEnd);

	std::string ToLower(const std::string& iString);
	std::wstring ToLower(const std::wstring& iString);

	void StringReplace(std::string* ioString, const std::string& iToFind, const std::string& iToReplace);
	void StringReplace(std::wstring* ioString, const std::wstring& iToFind, const std::wstring& iToReplace);
}

#endif