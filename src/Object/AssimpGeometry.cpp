#include "AssimpGeometry.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "../Misc/Links.h"

AssimpGeometry::AssimpGeometry(const std::string& fileName) {
	std::string path = MODELS_FOLDER + fileName;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
	if (scene == nullptr) {
		std::cerr << "Failed to load obj file " << path << ". Assimp error message: " << importer.GetErrorString();
		exit(1);
	}

	// for this example we only load the first mesh
	const aiMesh* mesh = scene->mMeshes[0];

	indices.reserve(mesh->mNumFaces*3);
	vertices.reserve(mesh->mNumVertices);

	glm::vec3 v;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		v.x = mesh->mVertices[i].x;
		v.y = mesh->mVertices[i].y;
		v.z = mesh->mVertices[i].z;

		vertices.emplace_back(v);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		indices.emplace_back(face.mIndices[0]);
		indices.emplace_back(face.mIndices[1]);
		indices.emplace_back(face.mIndices[2]);
	}
}
