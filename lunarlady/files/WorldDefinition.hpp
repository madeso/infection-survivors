#ifndef LLF_WORLD_DEFINITION_HPP
#define LLW_WORLD_DEFINITION_HPP

#include <string>
#include <map>
#include <vector>
#include <list>

#include "lunarlady/math/vec2.hpp"
#include "lunarlady/math/vec3.hpp"

namespace lunarlady {
	namespace files {
		enum WorldTypes {
			WT_SIMPLE
		};

		class Face;
		typedef std::vector<Face*> FacePtrList;
		class Vertex {
		public:
			Vertex(::lunarlady::real x, ::lunarlady::real y, ::lunarlady::real z) : loc(x,y,z), hasNormal(false), normalIndex(0) {
			}
			::lunarlady::math::vec3 loc;
			bool hasNormal;
			std::size_t normalIndex;
			FacePtrList faces;
		};

		//typedef ::lunarlady::math::vec3 Vertex;
		typedef ::lunarlady::math::vec3 Normal;
		typedef ::lunarlady::math::vec2 TexCoord;
		class Face {
		public:
			std::size_t vertex[3];
			std::size_t normal[3];
			std::size_t texcoord[3];

			const math::vec3& getNormal(const std::vector<Vertex>& vertices) const;

			Face() : mNormal(0,0,0), normalCalculated(false)  {
			}

			void setNormal(const Normal& iNormal);
		private:
			mutable Normal mNormal;
			mutable bool normalCalculated;
		};

		class World {
		public:
			void addProperty(const std::string& iName, const std::string& iValue);
			std::string getProperty(const std::string& iName);
		protected:
			typedef std::map<std::string, std::string> PropertyMap;
			PropertyMap mProperties;
		};

		typedef std::vector<Face> FaceList;
		typedef std::map<std::string, FaceList> FaceMap;

		enum WorldType {
			WT_SIMPLE_WORLD
		};

		class SimpleWorld : public World {
		public:
			void postProc();
			std::vector<Vertex> vertices;
			std::vector<Normal> normals;
			std::vector<TexCoord> texcoords;
			FaceMap faces;
		};

		namespace optimized {
			class Vertex {
			public:
				Vertex() : vertex(0,0,0), normal(0,0,0), texcoord(0,0) {
				}
				math::vec3 vertex;
				math::vec3 normal;
				math::vec2 texcoord;
			};
			class Face {
			public:
				Face() : normal(0,0,0) {
				}
				math::vec3 normal;
				std::size_t vertex[3];
			};
			typedef std::vector<Face> FaceList;
			typedef std::map<std::string, FaceList> FaceMap;
			class Model {
			public:
				explicit Model(const SimpleWorld& iWorld);
				Model() {}
				void load(char* iBuffer, std::size_t iSize);
				void save(const std::string& iFileName);

				std::vector<Vertex> vertices;
				FaceMap faces;
			};
		};
	}
}

#endif