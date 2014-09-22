#include "lunarlady/MaterialDefinitionContainer.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/Xml.hpp"

namespace lunarlady {
	MaterialDefinitionContainer::MaterialDefinitionContainer(const std::string& iFilePath) {
		std::vector<std::string> files;
		GetFileListing(iFilePath, &files);
		const std::size_t fileCount = files.size();
		for(std::size_t i=0; i<fileCount; ++i) {
			const std::string fileName = files[i];
			if( EndsWith(fileName, ".mdc") ) {
				ReadFile file(fileName);
				TiXmlDocument doc(fileName);
				doc.Parse(file.getBuffer(), 0, TIXML_ENCODING_LEGACY);
				const std::string baseName = "material";
				for(TiXmlElement* element=TiXmlHandle(&doc).FirstChildElement("materials").FirstChildElement(baseName).ToElement(); element; element = element->NextSiblingElement(baseName) ) {
					const std::string name = GetStringAttribute(element, baseName, "name", fileName);
					MaterialDefinition material(element);
					mDefinitions.insert( MaterialDefinitionMap::value_type(name, material) );
				}
			}
		}
	}
	const MaterialDefinition& MaterialDefinitionContainer::getMaterialDefinition(const std::string& iDefinitionName) const {
		MaterialDefinitionMap::const_iterator result = mDefinitions.find(iDefinitionName);
		if( result == mDefinitions.end() ) {
			throw "Failed to find material";
		}

		return result->second;
	}
}