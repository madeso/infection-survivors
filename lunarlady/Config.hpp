#ifndef LL_CONFIG_HPP
#define LL_CONFIG_HPP

#include "lunarlady/Types.hpp"

namespace lunarlady {
	int GetInt(const std::string& iName);
	real GetReal(const std::string& iName);
	bool GetBool(const std::string& iName);
	const std::string GetString(const std::string& iName);

	void SetInt(const std::string& iName, int iValue);
	void SetReal(const std::string& iName, real iValue);
	void SetBool(const std::string& iName, bool iValue);
	void SetString(const std::string& iName, const std::string& iValue);
}

#endif