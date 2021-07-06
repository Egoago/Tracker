#include "AssimpGeometry.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

AssimpGeometry::AssimpGeometry(const char* fileName) {
	std::string path = "Models/" + std::string(fileName);
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);
	if (scene == nullptr) {
		std::cerr << "Failed to load obj file " << path << ". Assimp error message: " << importer.GetErrorString();
		exit(1);
	}

	// for this example we only load the first mesh
	const aiMesh* mesh = scene->mMeshes[0];

	indices.reserve(mesh->mNumFaces*3);
	vertices.reserve(mesh->mNumVertices);

	Vertex v;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		v.position.x = mesh->mVertices[i].x;
		v.position.y = mesh->mVertices[i].y;
		v.position.z = mesh->mVertices[i].z;

		v.normal.x = mesh->mNormals[i].x;
		v.normal.y = mesh->mNormals[i].y;
		v.normal.z = mesh->mNormals[i].z;

		vertices.emplace_back(v);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		indices.emplace_back(face.mIndices[0]);
		indices.emplace_back(face.mIndices[1]);
		indices.emplace_back(face.mIndices[2]);
	}
}
