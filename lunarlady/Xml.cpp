#include "lunarlady/Xml.hpp"

#include "boost/lexical_cast.hpp"

#include "lunarlady/Error.hpp"
#include "lunarlady/StringUtils.hpp"
#include "UtfConverter.hpp"

namespace lunarlady {
	std::string StringAndLine(const std::string& iError, TiXmlElement* iElement) {
		std::ostringstream str;
		str << iError << ", row: " << iElement->Row();
		return str.str();
	}

	TiXmlElement& VerifyElement(TiXmlElement* iElement, const std::string& iElementName, const std::string& iFileName) {
		if( !iElement ) throw XmlError( std::string("Mising ") +  iElementName +  std::string(" in ") + StringAndLine(iFileName, iElement) );
		return *iElement;
	}
	const std::string VerifyAttribute(TiXmlElement& iElement, const std::string& iElementName, const std::string iAttributeName, const std::string iFileName) {
		const std::string* attribute = iElement.Attribute(iAttributeName);
		if( !attribute ) throw XmlError( std::string("Mising attribute ") +  iElementName + std::string( "::" ) + iAttributeName +  std::string(" in ") + StringAndLine(iFileName, &iElement) );
		return *attribute;
	}

	template <class T>
	T GetAttribute(TiXmlElement* iElement, const std::string& iElementName, const std::string& iAttributeName, const std::string& iFileName) {
		const std::string attribute = VerifyAttribute(VerifyElement(iElement, iElementName, iFileName), iElementName, iAttributeName, iFileName);
		T ret;
		try {
			ret = boost::lexical_cast<T>( attribute );
        }
		catch(boost::bad_lexical_cast &) {
			throw XmlError( std::string("Failed to convert ") +  iElementName + std::string( "::" ) + iAttributeName +  std::string(" to a valid type in ") + StringAndLine(iFileName, iElement) );
        }

		return ret;
	}

	int GetIntAttribute(TiXmlElement* iElement, const std::string& iElementName, const std::string& iAttributeName, const std::string& iFileName) {
		return GetAttribute<int>(iElement, iElementName, iAttributeName, iFileName);
	}

	std::string GetStringAttribute(TiXmlElement* iElement, const std::string& iElementName, const std::string& iAttributeName, const std::string& iFileName) {
		return GetAttribute<std::string>(iElement, iElementName, iAttributeName, iFileName);
	}

	std::wstring GetWStringAttribute(TiXmlElement* iElement, const std::string& iElementName, const std::string& iAttributeName, const std::string& iFileName) {
		const std::string str = GetAttribute<std::string>(iElement, iElementName, iAttributeName, iFileName);
		const std::wstring res = UtfConverter::FromUtf8(str);
		return res;
	}

	void SetWStringAttribute(TiXmlElement* iElement, const std::string& iAttributeName, const std::wstring& iValue) {
		const std::string value = UtfConverter::ToUtf8(iValue);
		iElement->SetAttribute(iAttributeName.c_str(), value.c_str());
	}

	std::wstring GetWStringText(TiXmlElement* iElement) {
		const char* str = iElement->GetText();
		if( !str ) throw XmlError( iElement->Value() + StringAndLine(" missing inner text", iElement) );
		return UtfConverter::FromUtf8(str);
	}

	void SetWStringText(TiXmlElement* iElement, const std::wstring& iValue) {
		const std::string value = UtfConverter::ToUtf8(iValue);
		iElement->InsertEndChild( TiXmlText(value) );
	}

	std::string GetStringText(TiXmlElement* iElement) {
		const char* str = iElement->GetText();
		if( !str ) throw XmlError( iElement->Value() + StringAndLine(" missing inner text", iElement) );
		return str;
	}
	void SetStringText(TiXmlElement* iElement, const std::string& iValue) {
		iElement->InsertEndChild( TiXmlText(iValue) );
	}
}