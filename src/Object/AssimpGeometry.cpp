#include "AssimpGeometry.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "../Misc/Links.h"
#include <glm/geometric.hpp>
#include "../Misc/Log.h"

using namespace tr;

AssimpGeometry::AssimpGeometry(const std::string& fileName) {
	Logger::logProcess(__FUNCTION__);
	std::string path = MODELS_FOLDER + fileName;
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
	indices.reserve(mesh->mNumFaces*3);
	vertices.reserve(mesh->mNumVertices);
	normals.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		vertices.emplace_back(mesh->mVertices[i].x,
							  mesh->mVertices[i].y,
							  mesh->mVertices[i].z);
		glm::vec3 normal(mesh->mNormals[i].x,
						 mesh->mNormals[i].y,
						 mesh->mNormals[i].z);
		normals.emplace_back(glm::normalize(normal));
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		indices.emplace_back(face.mIndices[0]);
		indices.emplace_back(face.mIndices[1]);
		indices.emplace_back(face.mIndices[2]);
	}
	detectEdgePairs();
	Logger::logProcess(__FUNCTION__);
}
