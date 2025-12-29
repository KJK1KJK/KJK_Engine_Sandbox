#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "Mesh.h"

class Model
{
public:
	//Constructor expecting a filepath to a 3D model
	Model(const std::string& path);

	~Model() = default;

	//Disable copy semantics
	Model(const Model& other) = delete;
	Model& operator=(const Model& other) = delete;

	//Allow move semantics
	Model(Model&& other) noexcept = default;
	Model& operator=(Model&& other) noexcept = default;

	//Draw all the meshes of the model
	void Draw(const Shader& shader) const;

	//Getter for meshes
	inline const std::vector<Mesh>& GetMeshes() const { return mMeshes; }
private:
	//Model data
	std::vector<Mesh> mMeshes;
	std::string mDirectory;

	//List of loaded textures to avoid loading duplicates
	std::vector<Texture> mLoadedTextures;

	//Load a model with all its meshes and textures
	bool loadModel(const std::string& path);

	//Recursively process a node in the scene graph
	void processNode(aiNode* node, const aiScene* scene);
	//Process a mesh
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	//Load material textures
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

	//Load a texture from file
	GLuint TextureFromFile(const char* path);
};