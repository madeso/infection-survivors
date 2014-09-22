#ifndef LL_MATERIAL_DEFINITION_HPP
#define LL_MATERIAL_DEFINITION_HPP

#include <string>
#include <vector>

#include "lunarlady/Rgb.hpp"
#include "lunarlady/Xml.hpp"

namespace lunarlady {
	class MaterialDefinition {
	public:
		MaterialDefinition(TiXmlElement* iElement);

		Rgb ambient;
		Rgb diffuse;
		Rgb emissive;
		Rgb specular;
		real exponent;
		real alpha;

		std::string colorTextureName;

		std::vector<std::string> textureMedia;

		void addMedia(const std::string& iMedia);
	};
}

#endif