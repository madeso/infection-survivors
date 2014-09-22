#ifndef LL_XML_HPP
#define LL_XML_HPP

#include "xml/tinyxml.h"
#include <string>

namespace lunarlady {
	int GetIntAttribute(TiXmlElement* iElement, const std::string& iElementName, const std::string& iAttributeName, const std::string& iFileName);
	std::string GetStringAttribute(TiXmlElement* iElement, const std::string& iElementName, const std::string& iAttributeName, const std::string& iFileName);
	std::wstring GetWStringAttribute(TiXmlElement* iElement, const std::string& iElementName, const std::string& iAttributeName, const std::string& iFileName);

	void SetWStringAttribute(TiXmlElement* iElement, const std::string& iAttributeName, const std::wstring& iValue);

	std::wstring GetWStringText(TiXmlElement* iElement);
	void SetWStringText(TiXmlElement* iElement, const std::wstring& iValue);

	std::string GetStringText(TiXmlElement* iElement);
	void SetStringText(TiXmlElement* iElement, const std::string& iValue);

	std::string StringAndLine(const std::string& iError, TiXmlElement* iElement);
}

#endif