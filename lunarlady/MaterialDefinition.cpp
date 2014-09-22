#include "lunarlady/MaterialDefinition.hpp"

#include <algorithm>

namespace lunarlady {

	namespace {
		real GetValue(TiXmlElement* iElement, real default) {
			real value = default;
			if( iElement ) {
				if( iElement->QueryDoubleAttribute("value", &value) == TIXML_WRONG_TYPE ) {
				}
			}
			return value;
		}
		Rgb GetColor(TiXmlElement* iElement, real gray) {
			Rgb result;
			real r=gray, g=gray, b=gray;

			if( iElement ) {
				real grayValue = 1;
				if( iElement->QueryDoubleAttribute("gray", &grayValue) == TIXML_SUCCESS ) {
					r = g = b = grayValue;
				}

				if( iElement->QueryDoubleAttribute("r", &r) == TIXML_WRONG_TYPE ) {
				}
				if( iElement->QueryDoubleAttribute("g", &g) == TIXML_WRONG_TYPE ) {
				}
				if( iElement->QueryDoubleAttribute("b", &b) == TIXML_WRONG_TYPE ) {
				}
			}

			result.setRed(r);
			result.setGreen(g);
			result.setBlue(b);

			return result;
		}
	}

	std::string GetInnerTextOrEmptyIfNull(TiXmlElement* iElement) {
		if( iElement ) {
			return GetStringText(iElement);
		}
		else {
			return "";
		}
	}

	MaterialDefinition::MaterialDefinition(TiXmlElement* iElement) {
		ambient = GetColor(iElement->FirstChildElement("ambient"), 0.2);
		diffuse = GetColor(iElement->FirstChildElement("diffuse"), 0.8);
		emissive = GetColor(iElement->FirstChildElement("emissive"), 0);
		specular = GetColor(iElement->FirstChildElement("specular"), 0);
		exponent = GetValue(TiXmlHandle(iElement).FirstChildElement("specular").FirstChildElement("exponent").ToElement(), 0);
		alpha = GetValue(iElement->FirstChildElement("alpha"), 1);

		TiXmlElement* texture = iElement->FirstChildElement("texture");
		if( texture ) {
			colorTextureName = GetInnerTextOrEmptyIfNull(texture->FirstChildElement("color"));
			addMedia( colorTextureName );
		}
	}
	void MaterialDefinition::addMedia(const std::string& iMedia) {
		if( iMedia.empty() ) return;
		if( std::find(textureMedia.begin(), textureMedia.end(), iMedia) == textureMedia.end() ) {
			textureMedia.push_back(iMedia);
		}
	}
}