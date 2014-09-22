#ifndef LL_MATERIAL_DEFINITION_CONTAINER_HPP
#define LL_MATERIAL_DEFINITION_CONTAINER_HPP

#include <map>
#include <string>
#include "lunarlady/MaterialDefinition.hpp"

namespace lunarlady {
	class MaterialDefinitionContainer {
	public:
		explicit MaterialDefinitionContainer(const std::string& iFilePath);

		const MaterialDefinition& getMaterialDefinition(const std::string& iDefinitionName) const;
	private:
		MaterialDefinitionContainer() {}
		typedef std::map<std::string, MaterialDefinition> MaterialDefinitionMap;
		MaterialDefinitionMap mDefinitions;
	};
}

#endif