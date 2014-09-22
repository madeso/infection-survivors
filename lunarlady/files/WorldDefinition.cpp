#include "lunarlady/files/WorldDefinition.hpp"
#include "lunarlady/math/Plane.hpp"
#include <fstream>
#include <strstream>
#include "boost/smart_ptr.hpp"

#define scope if(true)

namespace lunarlady {
	namespace files {
		const math::vec3& Face::getNormal(const std::vector<Vertex>& vertices) const {
			if( normalCalculated ) {
				return mNormal;
			}
			mNormal = ::lunarlady::math::Plane(vertices[vertex[0]].loc, vertices[vertex[1]].loc, vertices[vertex[2]].loc).getNormal();
			normalCalculated = true;
			return mNormal;
		}
		void Face::setNormal(const Normal& iNormal) {
			mNormal = iNormal;
			normalCalculated = true;
		}

		void Write(std::ofstream& file, std::size_t t) {
			file.write( reinterpret_cast<char*>(&t), sizeof(size_t));
		}
		std::size_t ReadSizeT(std::strstream& file) {
			std::size_t t=0;
			file.read( reinterpret_cast<char*>(&t), sizeof(size_t));
			return t;
		}
		void Write(std::ofstream& file, const std::string& str) {
			const std::size_t l = str.length();
			Write(file, l);
			file.write(str.c_str(), l);
		}
		std::string ReadString(std::strstream& file) {
			std::size_t l = ReadSizeT(file);
			boost::scoped_array<char> string(new char[l+1]);
			file.read(string.get(), l);
			string[l] = 0;
			return std::string(string.get());
		}
		void Write(std::ofstream& file, ::lunarlady::real r) {
			file.write(reinterpret_cast<char*>(&r), sizeof(::lunarlady::real));
		}
		::lunarlady::real ReadReal(std::strstream& file) {
			::lunarlady::real r=0;
			file.read(reinterpret_cast<char*>(&r), sizeof(::lunarlady::real));
			return r;
		}
		void SimpleWorld::postProc() {
			// fix up vertices and texcoords
			for( FaceMap::iterator faceIt = faces.begin(); faceIt != faces.end(); ++faceIt) {
				FaceList& faceList = (faceIt->second);
				// for each face
				for(FaceList::iterator face=faceList.begin(); face != faceList.end(); ++face) {
					// for each index
					for(int i=0; i<3; ++i) {
						face->vertex[i] -= 1;
						face->texcoord[i] -= 1;
					}
				}
			}

			// for each material
			for( FaceMap::iterator faceIt = faces.begin(); faceIt != faces.end(); ++faceIt) {
				FaceList& faceList = (faceIt->second);
				// for each face
				for(FaceList::iterator face=faceList.begin(); face != faceList.end(); ++face) {
					// for each index
					for(int i=0; i<3; ++i) {
						Vertex& vertex = vertices[face->vertex[i]];
						vertex.hasNormal = vertex.hasNormal || face->normal[i] != 0;
						vertex.faces.push_back( &(*face) );
					}
				}
			}

			const std::size_t vertexCount = vertices.size();
			for(std::size_t index=0; index<vertexCount; ++index) {
				Vertex& vertex = vertices[index];
				if( !vertex.hasNormal && vertex.normalIndex==0 ) {
					math::vec3 normal(0,0,0);
					for(FacePtrList::iterator face=vertex.faces.begin(); face!=vertex.faces.end(); ++face) {
						Face* f = *face;
						normal += f->getNormal(vertices);
					}
					//normal /= vertex.faces.size();
					normal.normalize();
					real length = normal.getLength();
					vertex.normalIndex = normals.size()+1;
					normals.push_back(normal);
				}
			}

			// for each material
			for( FaceMap::iterator faceIt = faces.begin(); faceIt != faces.end(); ++faceIt) {
				FaceList& faceList = (faceIt->second);
				// for each face
				for(FaceList::iterator face=faceList.begin(); face != faceList.end(); ++face) {
					// for each index
					for(int i=0; i<3; ++i) {
						Vertex& vertex = vertices[face->vertex[i]];
						if(! vertex.hasNormal ) {
							face->normal[i] = vertex.normalIndex;
						}
						face->normal[i] -= 1;
					}
				}
			}
		}
		void World::addProperty(const std::string& iName, const std::string& iValue) {
			mProperties[iName] = iValue;
		}
		std::string World::getProperty(const std::string& iName) {
			return mProperties[iName];
		}

		// -----------------
		namespace optimized {
			Model::Model(const SimpleWorld& iWorld) {
				struct Index {
					std::size_t vertex;
					std::size_t normal;
					std::size_t texcoord;
					Index(std::size_t v, std::size_t n, std::size_t t) : vertex(v), normal(n), texcoord(t) {
					}
					bool operator<(const Index& o) const {
						if( vertex != o.vertex ) {
							return vertex < o.vertex;
						}
						if( normal != o.normal ) {
							return normal < o.normal;
						}
						return texcoord < o.texcoord;
					}
				};

				typedef std::map<Index, std::size_t> Map;
				Map oldToNewIndices;
				std::size_t vertexIndex = 0;

				vertices.reserve( iWorld.vertices.size() );

				for(::lunarlady::files::FaceMap::const_iterator part=iWorld.faces.begin(); part != iWorld.faces.end(); ++part){
					const std::string materialName = part->first;
					::lunarlady::files::optimized::FaceList& toFaceList = faces[materialName];
					const ::lunarlady::files::FaceList& fromFaceList = part->second;
					const std::size_t length = fromFaceList.size();
					for(std::size_t faceIndex=0; faceIndex< length; ++faceIndex) {
						const ::lunarlady::files::Face& fromFace = fromFaceList[faceIndex];
						::lunarlady::files::optimized::Face toFace;
						toFace.normal = fromFace.getNormal(iWorld.vertices);
						for(std::size_t i=0; i<3; ++i) {
							Index index(fromFace.vertex[i], fromFace.normal[i], fromFace.texcoord[i]);
							Map::iterator result = oldToNewIndices.find(index);
							if( result == oldToNewIndices.end() ) {
								Vertex vertex;
								vertex.vertex = iWorld.vertices[ fromFace.vertex[i] ].loc;
								vertex.normal = iWorld.normals[ fromFace.normal[i] ];
								vertex.texcoord = iWorld.texcoords[ fromFace.texcoord[i] ];
								vertices.push_back(vertex);
								toFace.vertex[i] = vertexIndex;
								oldToNewIndices.insert( Map::value_type(index, vertexIndex) );
								++vertexIndex;
							}
							else {
								toFace.vertex[i] = result->second;
							}
						}
						toFaceList.push_back(toFace);
					}
				}
			}
			void Model::load(char* iBuffer, std::size_t iSize) {
				std::strstream file(iBuffer, iSize, std::ios::in | std::ios::binary);

				char id[2];
				file.read(id, sizeof(char)*2);
				ReadSizeT(file); // type
				ReadSizeT(file); // version

				scope {
					const std::size_t vertexCount = ReadSizeT(file);
					vertices.resize(vertexCount);
					for(std::size_t index=0; index<vertexCount; ++index) {
						scope {
							const real x = ReadReal(file);
							const real y = ReadReal(file);
							const real z = ReadReal(file);
							vertices[index].vertex = ::lunarlady::math::vec3(x,y,z);
						}
						scope {
							const real x = ReadReal(file);
							const real y = ReadReal(file);
							const real z = ReadReal(file);
							vertices[index].normal = Normal(x,y,z);
						}

						scope {
							const real x = ReadReal(file);
							const real y = ReadReal(file);
							vertices[index].texcoord = TexCoord(x,y);
						}
					}
				}

				scope {
					for(std::size_t materials=ReadSizeT(file); materials>0; --materials) {
						const std::string materialName = ReadString(file);
						FaceList& faceList = faces[materialName];
						const std::size_t faceCount = ReadSizeT(file);
						faceList.reserve(faceCount);
						for(std::size_t index=0; index<faceCount; ++index) {
							Face face;
							const real x = ReadReal(file);
							const real y = ReadReal(file);
							const real z = ReadReal(file);
							face.normal = math::vec3(x,y,z);
							for(std::size_t i=0; i<3; ++i) {
								face.vertex[i] = ReadSizeT(file);
							}
							faceList.push_back(face);
						}
					}
				}
			}
			void Model::save(const std::string& iFileName) {
				std::ofstream file(iFileName.c_str(), std::ios::out | std::ios::binary);
				if( !file.good() ) {
					throw "failed to open savefile";
				}

				file.write("ll", sizeof(char)*2);
				Write(file, (std::size_t) WT_SIMPLE_WORLD);
				Write(file, (std::size_t) 0);

				{ // vertices
					const std::size_t l = vertices.size();
					Write(file, l);
					for( std::size_t index=0; index < l; ++index) {
						Vertex& vertex = vertices[index];
						Write(file, vertex.vertex.getX());
						Write(file, vertex.vertex.getY());
						Write(file, vertex.vertex.getZ());

						Write(file, vertex.normal.getX());
						Write(file, vertex.normal.getY());
						Write(file, vertex.normal.getZ());

						Write(file, vertex.texcoord.getX());
						Write(file, vertex.texcoord.getY());
					}
				}

				{
					Write(file, faces.size());
					for(FaceMap::iterator matFace = faces.begin(); matFace != faces.end(); ++matFace) {
						Write(file, matFace->first);
						FaceList& faceList = matFace->second;
						const std::size_t l = faceList.size();
						Write(file, l);
						for( std::size_t index=0; index < l; ++index) {
							Face& face = faceList[index];
							const ::lunarlady::math::vec3 normal = face.normal;
							Write(file, normal.getX());
							Write(file, normal.getY());
							Write(file, normal.getZ());

							for(std::size_t index=0; index<3; ++index) {
								Write(file, face.vertex[index]);
							}
						}
					}
				}
			}
		}
	}
}