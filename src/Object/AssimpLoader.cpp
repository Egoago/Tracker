#include "AssimpLoader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Misc/Constants.hpp"
#include "../Misc/Log.hpp"

using namespace tr;

void tr::AssimpLoader::load(const std::string& fileName, Geometry& geometry) {
	Logger::logProcess(__FUNCTION__);
	std::string path = MODELS_FOLDER + fileName + MODELS_EXTENSION;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate
		| aiProcess_GenNormals
		//| aiProcess_FixInfacingNormals
		//| aiProcess_JoinIdenticalVertices
	);
	if (scene == nullptr) {
		Logger::error("Failed to load obj file " + path + ".Assimp error message : " + importer.GetErrorString());
		exit(1);
	}

	const aiMesh* mesh = scene->mMeshes[0];
	Logger::log("faces: " + std::to_string(mesh->mNumFaces));
	Logger::log("vertices: " + std::to_string(mesh->mNumVertices));
	geometry.indices.reserve(mesh->mNumFaces*3u);
	geometry.vertices.reserve(mesh->mNumVertices);
	geometry.normals.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		geometry.vertices.emplace_back(mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z);
		vec3f normal(mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z);
		geometry.normals.emplace_back(normal.matrix().normalized());
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		geometry.indices.emplace_back(face.mIndices[0]);
		geometry.indices.emplace_back(face.mIndices[1]);
		geometry.indices.emplace_back(face.mIndices[2]);
	}
	geometry.generate();
	Logger::logProcess(__FUNCTION__);
}
