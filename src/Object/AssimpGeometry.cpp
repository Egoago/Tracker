#include "AssimpGeometry.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//TODO remove logging
#include <iostream>
#include "../Misc/Links.h"

AssimpGeometry::AssimpGeometry(const std::string& fileName) {
	std::string path = MODELS_FOLDER + fileName;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate
		| aiProcess_GenNormals
		| aiProcess_FixInfacingNormals
		//| aiProcess_JoinIdenticalVertices
	);
	if (scene == nullptr) {
		std::cerr << "Failed to load obj file " << path << ". Assimp error message: " << importer.GetErrorString();
		exit(1);
	}

	const aiMesh* mesh = scene->mMeshes[0];

	std::cout << "faces: " << mesh->mNumFaces << std::endl;
	indices.reserve(mesh->mNumFaces*3);
	std::cout << "vertices: " << mesh->mNumVertices << std::endl;
	vertices.reserve(mesh->mNumVertices);
	normals.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		vertices.emplace_back(mesh->mVertices[i].x,
							mesh->mVertices[i].y,
							mesh->mVertices[i].z);
		normals.emplace_back(mesh->mNormals[i].x,
							mesh->mNormals[i].y,
							mesh->mNormals[i].z);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		indices.emplace_back(face.mIndices[0]);
		indices.emplace_back(face.mIndices[1]);
		indices.emplace_back(face.mIndices[2]);
	}
	detectEdgePairs();
}
