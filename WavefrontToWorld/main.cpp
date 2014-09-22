#include <iostream>
#include <fstream>
#include "boost/lexical_cast.hpp"
#include "lunarlady/files/WorldDefinition.hpp"
#include "lunarlady/StringUtils.hpp"

typedef ::lunarlady::real real;

void GetData(const std::string& text, std::string& a, std::string& b, std::string& c) {
	std::ostringstream stream[3];
	std::size_t str = 0;
	const std::size_t l = text.length();

	for(std::size_t i=0; i<l; ++i) {
		const char ch = text[i];
		if( ch=='/' ) {
			++str;
		}
		else if( str < 3 ) {
			stream[str] << ch;
		}
	}

	a = ::lunarlady::Trim(stream[0].str());
	b = ::lunarlady::Trim(stream[1].str());
	c = ::lunarlady::Trim(stream[2].str());
}

std::size_t GrabIndex(std::string val, std::size_t max) {
	if( val.empty() ) return 0;
	typedef long type;
	const type index = boost::lexical_cast<type> (val);
	if( index < 0 ) {
		return max + index +1;
	}
	else {
		return index;
	}
}

void ReadIndex(::lunarlady::files::Face& face, const std::string& iData, std::size_t index, std::size_t vertexCount, std::size_t normalCount, std::size_t texCount) {
	const std::string data = ::lunarlady::Trim(iData);
	std::string vertex;
	std::string texcoord;
	std::string normal;
	GetData(iData, vertex, texcoord, normal);
	face.vertex[index] = GrabIndex(vertex, vertexCount);
	face.texcoord[index] = GrabIndex(texcoord, texCount);
	face.normal[index] = GrabIndex(normal, normalCount);
}
void CopyFromTo(::lunarlady::files::Face& face, std::size_t from, std::size_t to) {
#define Prop(pr) face.pr[to] = face.pr[from]
	Prop(vertex);
	Prop(normal);
	Prop(texcoord);
#undef Prop
}

void main(int argc, char *argv[]) {
	if( argc != 2 ) {
		std::cout << "Usage: " << std::endl
			<< argv[0] << " obj-file" << std::endl
			<< std::endl
			<< "Press any key to continue..." << std::endl;
		std::cin.get();
		return;
	}

	std::ifstream file(argv[1]);
	std::string outFileName = std::string(argv[1]) + ".wrld";
	std::string extraMaterialName = "";
	if( !file.good() ) {
		std::cerr << "Failed to open file: " << argv[1] << std::endl;
		return;
	}

	::lunarlady::files::SimpleWorld world;

	world.addProperty("input", argv[1]);
	world.addProperty("output", outFileName);
	world.addProperty("extra material", extraMaterialName);

	std::string line;
	std::size_t indexBaseVertex = 0;
	std::size_t indexBaseNormal = 0;
	std::size_t indexBaseTex = 0;


	std::size_t lineno = 0;
	std::size_t facecount = 0;
	std::size_t tricount = 0;
	std::cout << "working..." << std::endl;
	::lunarlady::files::FaceList* faces = 0;
	while( std::getline(file, line) ) {
		++lineno;

		if( lineno %1500 ==0 ) {
			std::cout << "still working: " << lineno << std::endl;
		}

		std::vector<std::string> data;
		::lunarlady::SplitString(" ", line, &data);
		if( data.size() > 0 ) {
			const std::string id = ::lunarlady::ToLower( ::lunarlady::Trim(data[0]) );
			if( id == "o" ) {
				indexBaseVertex = world.vertices.size();
				indexBaseNormal = world.normals.size();
				indexBaseTex = world.texcoords.size();
			}
			else if( id == "v" ) {
				if( data.size() == 4) {
					const real x = boost::lexical_cast<real>(data[1]);
					const real y = boost::lexical_cast<real>(data[2]);
					const real z = boost::lexical_cast<real>(data[3]);
					world.vertices.push_back(::lunarlady::files::Vertex(x,y,z));
				}
				else {
					std::cerr << "Vertex on line: " << line << " (" << lineno << ") is bad format" << std::endl;
				}
			}
			else if( id == "vn" ) {
				if( data.size() == 4) {
					const real x = boost::lexical_cast<real>(data[1]);
					const real y = boost::lexical_cast<real>(data[2]);
					const real z = boost::lexical_cast<real>(data[3]);
					world.normals.push_back(::lunarlady::files::Normal(x,y,z));
				}
				else {
					std::cerr << "Normal on line: " << line << " (" << lineno << ") is bad format" << std::endl;
				}
			}
			else if( id == "vt" ) {
				if( data.size() == 3 || data.size() == 4) {
					const real x = boost::lexical_cast<real>(data[1]);
					const real y = boost::lexical_cast<real>(data[2]);
					world.texcoords.push_back(::lunarlady::files::TexCoord(x,y));
				}
				else {
					std::cerr << "TexCoord on line: " << line << " (" << lineno << ") is bad format" << std::endl;
				}
			}
			else if( id == "usemtl") {
				if( data.size() == 2) {
					faces = &(world.faces[extraMaterialName + data[1]]);
				}
				else {
					std::cerr << "UseMtl on line: " << line << " (" << lineno << ") is bad format" << std::endl;
				}
			}
			else if( id == "f" ) {
				if( faces ) {
					++facecount;
					if( data.size() > 3) {
						::lunarlady::files::Face face;
						std::size_t vertexCount = world.vertices.size();
						std::size_t normalCount = world.normals.size();
						std::size_t texCount = world.texcoords.size();
						ReadIndex(face, data[1], 0, vertexCount, normalCount, texCount);
						ReadIndex(face, data[2], 2, vertexCount, normalCount, texCount);
						for(std::size_t index=3; index<data.size(); ++index) {
							++tricount;							// copy 2 to 1
							CopyFromTo(face, 2, 1);
							ReadIndex(face, data[index], 2, vertexCount, normalCount, texCount);
							faces->push_back(face);
						}
					}
					else {
						std::cerr << "f on line: " << line << " (" << lineno << ") s a bad format" << std::endl;
					}
				}
				else {
					std::cerr << "f on line: " << line << " (" << lineno << ") missing mtl" << std::endl;
				}
			}
		}
	}

	std::cout << "Post processings... " << std::endl;
	world.postProc();

	std::cout << "Optimizing for fast display... " << std::endl;
	::lunarlady::files::optimized::Model model(world);
	std::cout << "Saving file... " << std::endl;
	model.save(outFileName);

	std::cout << "Vertices: " << world.vertices.size() << std::endl
			  << "Normals: " << world.normals.size() << std::endl
			  << "TexCoords: " << world.texcoords.size() << std::endl
			  << "Optimized count: " << model.vertices.size() << std::endl
			  << "Faces: " << facecount << std::endl
			  << "Optimezed percentage: ";
	std::cout.precision(2);
	std::cout.width(2);
	std::cout << std::fixed << std::showpoint 
			  << 100* static_cast<float>(model.vertices.size()) / static_cast<float>(tricount*3) << "%"<< std::endl
			  << "Tris: " << tricount << std::endl
			  << "Materials: " << world.faces.size() << std::endl;
	std::cin.get();
}