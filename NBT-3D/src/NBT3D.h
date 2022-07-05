#pragma once
// Includes GL stuff, with some helping functions. Includes GLM
#ifndef NBT3D_H
#define NBT3D_H
#include <vector>
#include "glhelper.h"
#include "../nbt/nbt.hpp"
#include "../loadgz/gznbt.h"
#include "stb_image_write.h"
#include <string>
#include <fstream>
#include <sstream>

#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <boost/stacktrace.hpp>

constexpr unsigned int hash(const char* s, int off = 0) {
	return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
}
namespace nbt {
	using namespace glm;
	struct MTL { // Wrapper for MTL Material type
		vec3 Ka = vec3(0); // Ambient color
		unsigned int mapKa = 0;
		vec3 Kd = vec3(0); // Diffuse color
		unsigned int mapKd = 0;
		vec3 Ks = vec3(0); // Specular color
		unsigned int mapKs = 0;
		vec3 Ke = vec3(0); // Emissive color
		unsigned int mapKe = 0;
		float Ns = 0; // Specular Exponent
		unsigned int mapNs = 0;
		float d = 1; // Transparency 1 = opaque
		unsigned int mapd = 0;
		float Ni = 0; // Index of refraction
		float sharpness = 60;
		vec3 Tf = vec3(0); // Transmission filter
		int illum = 0; // Illumination mode, https://en.wikipedia.org/wiki/Wavefront_.obj_file
		unsigned int bump = 0;
		unsigned int disp = 0;
		unsigned int refl = 0;
	};
	class NBT3D {
	public:
		std::vector<vec3> points = std::vector<vec3>();
		std::vector<vec2> uv = std::vector<vec2>();
		std::vector<vec3> normal = std::vector<vec3>();
		std::vector<u32vec3> faces = std::vector<u32vec3>();
		MTL material = MTL();
		NBT3D() {};

		std::vector<double> getPointData() {
			std::vector<double> pointData = std::vector<double>();
			for (auto i = points.begin(); i!=points.end(); i++) {
				pointData.push_back(i->x);
				pointData.push_back(i->y);
				pointData.push_back(i->z);
			}
			return pointData;
		}
		std::vector<double> getPointData4D() {
			std::vector<double> pointData = std::vector<double>();
			for (auto i = points.begin(); i != points.end(); i++) {
				pointData.push_back(i->x);
				pointData.push_back(i->y);
				pointData.push_back(i->z);
				pointData.push_back(1);
			}
			return pointData;
		}
		std::vector<double> getPointTextureData() {
			std::vector<double> data = std::vector<double>();
			for (int i = 0; i < points.size() && i < uv.size(); i++) {
				data.push_back(points[i].x);
				data.push_back(points[i].y);
				data.push_back(points[i].z);
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
			return data;
		}
		std::vector<double> getPointTextureData4D() {
			std::vector<double> data = std::vector<double>();
			for (int i = 0; i < points.size() && i < uv.size(); i++) {
				data.push_back(points[i].x);
				data.push_back(points[i].y);
				data.push_back(points[i].z);
				data.push_back(1);
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
			return data;
		}
		std::vector<float> getPointTextureNormalData() {
			std::vector<float> data = std::vector<float>();
			for (int i = 0; i < points.size(); i++) {
				data.push_back(points[i].x);
				data.push_back(points[i].y);
				data.push_back(points[i].z);
				if (uv.size() > i) {
					data.push_back(uv[i].x);
					data.push_back(uv[i].y);
				}
				else {
					data.push_back(0);
					data.push_back(0);
				}
				data.push_back(normal[i].x);
				data.push_back(normal[i].y);
				data.push_back(normal[i].z);
			}
			return data;
		}
		std::vector<unsigned int> getFaceIndiceData() {
			std::vector<unsigned int> data = std::vector<unsigned int>();
			for (auto i = faces.begin(); i != faces.end(); i++) {
				data.push_back(i->x);
				data.push_back(i->y);
				data.push_back(i->z);
			}
			return data;
		}

		void writeNBT(std::string filepath) {
			compound outc = compound();
			list plist = list("points");
			plist.tag_type = 5;
			for (size_t i = 0; i < points.size(); i++) {
				plist << new floattag(points[i].x);
				plist << new floattag(points[i].y);
				plist << new floattag(points[i].z);
			}
			list nlist = list("normals");
			for (size_t i = 0; i < normal.size(); i++) {
				nlist << new floattag(normal[i].x);
				nlist << new floattag(normal[i].y);
				nlist << new floattag(normal[i].z);
			}
			list ulist = list("uv");
			for (size_t i = 0; i < uv.size(); i++) {
				ulist << new floattag(uv[i].x);
				ulist << new floattag(uv[i].y);
			}
			list flist = list("faces");
			for (size_t i = 0; i < faces.size(); i++) {
				flist << new ulongtag(faces[i].x);
				flist << new ulongtag(faces[i].y);
				flist << new ulongtag(faces[i].z);
			}

			compound mat = compound("material");
			mat << new list("diffuse color", std::vector<tag_p>({ new floattag(material.Kd.x), new floattag(material.Kd.y), new floattag(material.Kd.z) }));
			mat << new list("ambient color", std::vector<tag_p>({ new floattag(material.Ka.x), new floattag(material.Ka.y), new floattag(material.Ka.z) }));
			mat << new list("specular color", std::vector<tag_p>({ new floattag(material.Ks.x), new floattag(material.Ks.y), new floattag(material.Ks.z) }));
			mat << new list("emissive color", std::vector<tag_p>({ new floattag(material.Ke.x), new floattag(material.Ke.y), new floattag(material.Ke.z) }));
			mat << new floattag("specular exponent", material.Ns);
			//stbi_write_png_compression_level = 9;			glActiveTexture(GL_TEXTURE0);
			auto mapping = [&](std::string type, unsigned int texture) {
				int width = 0, height = 0;
				glBindTexture(GL_TEXTURE_2D, texture);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
				unsigned char* pixels = new unsigned char[width * height * 4];
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
				stbi_flip_vertically_on_write(true);
				stbi_write_png("tmp.png", width, height, 4, pixels, width * 4);
				std::ifstream in = std::ifstream("tmp.png", std::ios::binary);
				in.seekg(0, std::ios::end);
				std::vector<int8_t> bytes = std::vector<int8_t>(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read((char*)&bytes[0], bytes.size());
				in.close();
				remove("tmp.png");
				mat << new bytearray(type, bytes);
			};
			if (material.mapKa != 0)
				mapping("ambient map", material.mapKa);
			if (material.mapKd != 0)
				mapping("diffuse map", material.mapKd);
			if (material.mapKs != 0)
				mapping("specular map", material.mapKs);
			if (material.mapKe != 0)
				mapping("emissive map", material.mapKe);
			if (material.bump != 0)
				mapping("bump map", material.bump);
			if (material.disp != 0)
				mapping("displacement map", material.disp);
			 
			outc << &mat;

			outc << &plist;
			outc << &nlist;
			outc << &ulist;
			outc << &flist;

			std::vector<char> out = std::vector<char>();
			outc.write(out);
			std::vector<char> outdef = std::vector<char>(); 
			deflate(&out[0], out.size(), &outdef, 9);
			std::ofstream outstream = std::ofstream(filepath, std::ios::binary);
			outstream.write(&outdef[0], outdef.size());
			outstream.close();
		}

		static std::vector<NBT3D> loadNBT(std::string filepath) {
			std::vector<NBT3D> objects = std::vector<NBT3D>();

			std::ifstream in(filepath, std::ios::binary);
			in.seekg(0, std::ios::end);
			std::vector<char> inb = std::vector<char>(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&inb[0], inb.size());
			in.close();
			std::vector<char> oub = std::vector<char>();
			inflate(&inb[0], inb.size(), &oub);
			compound loaded = compound();
			loaded.load(&oub[0], 0);

			NBT3D out = NBT3D();
			list pointl = loaded["points"].li();
			list normall = loaded["normals"].li();
			list uvl = loaded["uv"].li();
			list facesl = loaded["faces"].li();
			for (size_t i = 0; i < pointl.tags.size(); i+=3)
				out.points.push_back(vec3(pointl[i].f(), pointl[i+1].f(), pointl.tags[i + 2].f()));
			for (size_t i = 0; i < normall.tags.size(); i += 3)
				out.normal.push_back(vec3(normall[i].f(), normall[i + 1].f(), normall.tags[i + 2].f()));
			for (size_t i = 0; i < uvl.tags.size(); i += 2)
				out.uv.push_back(vec2(uvl[i].f(), uvl[i + 1].f()));
			for (size_t i = 0; i < facesl.tags.size(); i += 3)
				out.faces.push_back(u32vec3(facesl[i].l(), facesl[i + 1].l(), facesl.tags[i + 2].l()));

			compound materials = loaded["material"].c();
			
			// Converts loaded png binary data to a OpenGL Texture.
			auto toTexture = [&](unsigned char* data, size_t size, GLenum mapping) {
				unsigned int texture;						// Have empty number,
				glGenTextures(1, &texture);					// Gen texture id into that number,
				glBindTexture(GL_TEXTURE_2D, texture);		// Bind it

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);			// Repeat/clip/etc image horizontally
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);			// Repeat/clip/etc image vertically
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapping);	// Set interpolation at minimum
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mapping);	// Set interpolation at maximum

				int width, height, nrChannels;

				unsigned char* cdata = stbi_load_from_memory(data, size, &width, &height, &nrChannels, STBI_rgb_alpha); // Load the texture and get width, height, and channels number
				if (data) {
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, cdata);	// Buffer data into the texture
					glGenerateMipmap(GL_TEXTURE_2D);														// Generate mipmap based on interpolation
				}
				stbi_image_free(cdata);					// Empty data,
				return texture;							// and return the texture
			};

			for (auto tag = materials.tags.begin(), end = materials.tags.end(); tag != end; ++tag) {
				std::string name = tag->second->name;
				switch (hash(name.c_str())) {
				case hash("ambient color"): {
					auto Ka = tag->second.li().tags;
					out.material.Ka = vec3(Ka[0].f(), Ka[1].f(), Ka[2].f());
					break;
				}
				case hash("diffuse color"): {
					auto Kd = tag->second.li().tags;
					out.material.Kd = vec3(Kd[0].f(), Kd[1].f(), Kd[2].f());
					break;
				}
				case hash("specular color"): {
					auto Ks = tag->second.li().tags;
					out.material.Ks = vec3(Ks[0].f(), Ks[1].f(), Ks[2].f());
					break;
				}
				case hash("emissive color"): {
					auto Ke = tag->second.li().tags;
					out.material.Ke = vec3(Ke[0].f(), Ke[1].f(), Ke[2].f());
					break;
				}
				case hash("specular exponent"): {
					out.material.Ns = tag->second.f();
					break;
				}
				case hash("diffuse map"): {
					auto mapKd = tag->second.ba();
					out.material.mapKd = toTexture((unsigned char*)&mapKd[0], mapKd.size(), GL_LINEAR);
					break;
				}
				}
			}

			objects.push_back(out);
			loaded.discard();

			return objects;
		}

		static std::vector<NBT3D> loadOBJ(std::string filepath) {
			if (filepath.find(".nbt") != std::string::npos)
				return loadNBT(filepath);
			std::vector<NBT3D> objects = std::vector<NBT3D>();
			std::map<std::string, MTL> materials = std::map<std::string, MTL>();
			std::map<std::string, MTL> nmat = std::map<std::string, MTL>();
			std::string line;
			std::ifstream in(filepath);

			std::vector<vec3> points = std::vector<vec3>();
			std::vector<vec2> uv = std::vector<vec2>();
			std::vector<vec3> normal = std::vector<vec3>();
			std::vector<u32vec3> indices = std::vector<u32vec3>(); // .x = points, .y = uv, .z = normal

			//std::hash<std::string> hash = std::hash<std::string>();
			while (std::getline(in, line)) {
				std::vector<std::string> tokens = split(line, " ");
				if (tokens.size() == 0)
					continue;
				switch (hash(tokens[0].c_str())) {
				case hash("#"):
				case hash("g"): // Unsupported
				case hash("vp"): // Unsupported
				case hash("l"): // Unsupported
				default: // Probably Unsupported
					continue;
				case hash("o"):
					objects.push_back(NBT3D());
					points.clear();
					uv.clear();
					normal.clear();
					break;
				case hash("v"):
					points.push_back(vec3(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3])));
					break;
				case hash("vn"):
					normal.push_back(normalize(vec3(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]))));
					break;
				case hash("vt"):
					uv.push_back(vec2(std::stod(tokens[1]), std::stod((tokens.size()>1) ? tokens[2] : "0.0")));
					break;
				case hash("f"):
					if (tokens.size() < 2)
						break;
					if (objects.size() == 0)
						objects.push_back(NBT3D());
					// Create temporary face (could have > 3 verts)
					indices = std::vector<u32vec3>(); // .x = points, .y = uv, .z = normal
					tokens.erase(tokens.begin());
					for (int i = 0; i < tokens.size(); i++) {
						if (i % 3 == 1 && i > 1) { // i = 4, 7, 10.. (copy last two indices to the end)
							indices.push_back(indices[indices.size() - 2]);
							indices.push_back(indices[indices.size() - 2]);
						}
						if (tokens[i].find("/") == std::string::npos) { // f v1 v2 v3 ...
							u32vec3 index = u32vec3();
							index.x = std::stoi(tokens[i]);
							index.y = 0;
							index.z = 0;
							indices.push_back(index);
						}
						else {
							std::vector<std::string> sind = split(tokens[i], "/");
							if (sind.size() == 2) { // v1 / vt1
								u32vec3 index = u32vec3();
								index.x = std::stoi(sind[0]); // v1
								index.y = 0;
								index.z = std::stoi(sind[1]);
								indices.push_back(index);
							}
							else { // f v1/vt1?/vn1
								u32vec3 index = u32vec3();
								index.x = std::stoi(sind[0]); // v1
								if (sind[1].size() == 0) // f v1//vn1
									index.y = 0;
								else index.y = std::stoi(sind[1]);
								index.z = std::stoi(sind[2]);
								indices.push_back(index);
							}
						}
					}
					for (size_t i = 0; i + 2ll < indices.size(); i+=3) {
						objects[objects.size() - 1].addFace(
							points[indices[i].x-1],
							points[indices[i + 1ll].x-1],
							points[indices[i + 2ll].x-1],
							uv[indices[i].y-1],
							uv[indices[i + 1ll].y-1],
							uv[indices[i + 2ll].y-1],
							normal[indices[i].z-1],
							normal[indices[i + 1ll].z-1],
							normal[indices[i + 2ll].z-1]
						);
					}
					break;
				case hash("mtllib"):
					nmat = loadMaterials(tokens[1]);
					materials.insert(nmat.begin(), nmat.end());
					break;
				case hash("usemtl"):
					objects[objects.size() - 1].material = materials[tokens[1]];
					break;
				}
			}
			in.close();
			return objects;
		}

		static std::map<std::string, MTL> loadMaterials(std::string filepath) {
			std::map<std::string, MTL> materials = std::map<std::string, MTL>();
			std::string line;
			std::ifstream in = std::ifstream(filepath);

			MTL* mt = new MTL();

			while (std::getline(in, line)) {
				std::vector<std::string> tokens = split(line, " ");
				if (tokens.size() == 0)
					continue;
				auto catchVec3 = [tokens]() {
					if (tokens[1].find("xyz") != std::string::npos || tokens[1].find("spectral") != std::string::npos) {
						std::cerr << "CIEXYZ and Spectral RFL files not supported!" << std::endl << boost::stacktrace::stacktrace() << std::endl;
						return vec3(0);
					}
					return vec3(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]));
				};

				switch (hash(tokens[0].c_str())) {
				default:
					continue;
				case hash("newmtl"):
					materials.insert(std::make_pair(tokens[1], MTL()));
					mt = &materials[tokens[1]];
					break;
				case hash("Ka"):
					mt->Ka = catchVec3();
					break;
				case hash("Kd"):
					mt->Kd = catchVec3();
					break;
				case hash("Ks"):
					mt->Ks = catchVec3();
					break;
				case hash("Ke"):
					mt->Ke = catchVec3();
					break;
				case hash("Tf"):
					mt->Tf = catchVec3();
					break;
				case hash("illum"):
					mt->illum = std::stoi(tokens[1]);
					break;
				case hash("d"):
					mt->d = std::stod(tokens[1]);
					break;
				case hash("Tr"):
					mt->d = 1-std::stod(tokens[1]);
					break;
				case hash("Ns"):
					mt->Ns = std::stod(tokens[1]);
					break;
				case hash("Ni"):
					mt->Ni = std::stod(tokens[1]);
					break;
				case hash("sharpness"):
					mt->sharpness = std::stod(tokens[1]);
					break;
				case hash("map_Ka"):
					mt->mapKa = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				case hash("map_Kd"):
					mt->mapKd = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				case hash("map_Ks"):
					mt->mapKs = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				case hash("map_Ns"):
					mt->mapNs = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				case hash("map_d"):
					mt->mapd = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				case hash("disp"):
					mt->disp = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				case hash("map_bump"):
				case hash("bump"):
					mt->bump = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				case hash("refl"):
					mt->refl = getTextureFrom(tokens[tokens.size() - 1].c_str());
					break;
				}
			}
			return materials;
		}

		void addFace(vec3 a, vec3 b, vec3 c) {
			auto find = [&](vec3 in) {
				auto i = points.begin();
				if ((i = std::find(points.begin(), points.end(), in)) != points.end()) {
					return (unsigned long long) (i - points.begin());
				}
				else {
					points.push_back(in);
					return points.size() - 1;
				}
			};
			unsigned int ain = find(a);
			unsigned int bin = find(b);
			unsigned int cin = find(c);
			faces.push_back(u32vec3(ain, bin, cin));
		}
		void addFace(vec3 a, vec3 b, vec3 c, vec2 auv, vec2 buv, vec2 cuv) {
			auto find = [&](vec3 in, vec2 uin) {
				auto i = points.begin();
				auto j = uv.begin();
				if ((i = std::find(points.begin(), points.end(), in)) != points.end() && std::find(uv.begin(), uv.end(), uin)!=uv.end()) {
					return (unsigned long long) (i - points.begin());
				}
				else {
					points.push_back(in);
					uv.push_back(uin);
					return points.size() - 1;
				}
			};
			unsigned int ain = find(a, auv);
			unsigned int bin = find(b, buv);
			unsigned int cin = find(c, cuv);
			faces.push_back(u32vec3(ain, bin, cin));
		}
		void addFace(vec3 a, vec3 b, vec3 c, vec2 auv, vec2 buv, vec2 cuv, vec3 an, vec3 bn, vec3 cn) {
			auto find = [&](vec3 in, vec2 uin, vec3 nin) {
				auto i = points.begin();
				auto j = uv.begin();
				auto k = normal.begin();
				if ((i = std::find(points.begin(), points.end(), in)) != points.end() && (j = std::find(uv.begin(), uv.end(), uin)) != uv.end() && (k = std::find(normal.begin(), normal.end(), nin)) != normal.end() && (size_t)(i - points.begin()) == (size_t)(j - uv.begin()) && (size_t)(j - uv.begin()) == (size_t)(k - normal.begin())) {
					return (unsigned long long) (i - points.begin());
				}
				else {
					points.push_back(in);
					uv.push_back(uin);
					normal.push_back(nin);
					return points.size() - 1;
				}
			};
			unsigned int ain = find(a, auv, an);
			unsigned int bin = find(b, buv, bn);
			unsigned int cin = find(c, cuv, cn);
			faces.push_back(u32vec3(ain, bin, cin));
			//faces.push_back(u32vec3(points.size(), points.size() + 1, points.size() + 2));
			//points.push_back(a);
			//points.push_back(b);
			//points.push_back(c);
			//uv.push_back(auv);
			//uv.push_back(buv);
			//uv.push_back(cuv);
			//normal.push_back(an);
			//normal.push_back(bn);
			//normal.push_back(cn);
		}
		void calculateNormals() {
			if (normal.size() != 0)
				normal.clear();
			normal.insert(normal.begin(), points.size(), vec3(2));
			for (int i = 0; i < faces.size(); i++) {
				u32vec3& face = faces[i];
				// Check if face's vertices already have normals.
				// For any that do, duplicate them to the end of points (and uv if exists) and create a new normal, changing the indice in face
				if (normal[face.x].x == 2) { // Vertice 1 already has normal
					points.push_back(vec3(points[face.x]));
					if(uv.size()>face.x)
						uv.push_back(vec2(uv[face.x]));
					normal.push_back(vec3(2));
				}
				if (normal[face.y].x == 2) { // Vertice 1 already has normal
					points.push_back(vec3(points[face.y]));
					if (uv.size() > face.y)
						uv.push_back(vec2(uv[face.y]));
					normal.push_back(vec3(2));
				}
				if (normal[face.z].x == 2) { // Vertice 1 already has normal
					points.push_back(vec3(points[face.z]));
					if (uv.size() > face.z)
						uv.push_back(vec2(uv[face.z]));
					normal.push_back(vec3(2));
				}

				// Calculate normal of triangle
				vec3 norm = vec3();
				vec3 p1 = points[face.x];
				vec3 p2 = points[face.y];
				vec3 p3 = points[face.z];
				vec3 A = p2 - p1;
				vec3 B = p3 - p1;
				norm.x = A.y * B.z - A.z * B.y;
				norm.y = A.z * B.x - A.x * B.z;
				norm.z = A.x * B.y - A.y * B.x;
				norm = glm::normalize(norm);
				normal[face.x] = norm;
				normal[face.y] = norm;
				normal[face.z] = norm;
			}
		}
		// Only useful for my system, might not be very malleable towards other rendering systems.
		unsigned int getVAO() {
			std::vector<float> vertices = getPointTextureNormalData();
			std::vector<unsigned int> indices = getFaceIndiceData();

			unsigned int VAO;
			glGenVertexArrays(1, &VAO);
			unsigned VBO = 0, EBO = 0;
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size() , &vertices[0], GL_STATIC_DRAW);
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
			return VAO;
		}
		void uniformMaterial(unsigned int shader) {
			uniF3(shader, "material.Ka", material.Ka);
			uniF3(shader, "material.Ke", material.Ke);
			uniF3(shader, "material.Kd", material.Kd);
			uniI1(shader, "material.mapKd", 2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, material.mapKd);
			uniB(shader, "material.mapKdExists", material.mapKd != 0);
			glActiveTexture(GL_TEXTURE0);
			uniF3(shader, "material.Ks", material.Ks);
			uniF1(shader, "material.Ns", material.Ns);
			uniF1(shader, "material.d", material.d);
		}

	private:
		static std::vector<std::string> split(std::string s, std::string delimiter) {
			size_t pos = 0;
			std::vector<std::string> out = std::vector<std::string>();
			std::string token;
			while ((pos = s.find(delimiter)) != std::string::npos) {
				token = s.substr(0, pos);
				out.push_back(token);
				s.erase(0, pos + delimiter.length());
			}
			out.push_back(s);
			return out;
		}
	};
}
#endif