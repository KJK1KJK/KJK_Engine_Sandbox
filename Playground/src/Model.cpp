#include "Model.h"
#include <KJK_Engine/Core/Logger.h>

Model::Model(const std::string& path)
{
	loadModel(path);

	KJK_INFO("Loaded model at path: {0}", path.c_str());
	KJK_INFO("Model contains: {0} meshes", std::to_string(mMeshes.size()));
}

void Model::Draw(const Shader& shader) const
{
	//Draw each mesh in the model
	for (const auto& mesh : mMeshes)
	{
		mesh.Draw(shader);
	}
}

bool Model::loadModel(const std::string& path)
{
	//Create an instance of the Assimp Importer class
	Assimp::Importer importer;

	//Read the model file into a scene object
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	//Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		KJK_ERROR("ASSIMP error: {0}", importer.GetErrorString());
		return false;
	}

	//Retrieve the directory path of the filepath
	mDirectory = path.substr(0, path.find_last_of('/'));

	//Process the root node recursively
	processNode(scene->mRootNode, scene);

	return true;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	//Process each mesh located at the current node
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		mMeshes.push_back(processMesh(mesh, scene));
	}

	//Recursively process each of the child nodes
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	//Declare vectors to hold the mesh data
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	Material material{};

	//Iterate over each vertex
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		//Create a vertex object
		Vertex vertex{};
		glm::vec3 vector{};
		//Process vertex positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;
		//Process vertex normals
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;
		}
		//Process texture coordinates
		if (mesh->mTextureCoords[0])
		{
			//Declare a vec2 to hold the texture coordinates
			glm::vec2 vec{};
			//Get the texture coordinates from the first set
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = vec;
			//Process tangent
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.tangent = vector;
			//Process bitangent
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.bitangent = vector;
		}
		else
		{
			//If no texture coordinates, set to default value
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}

		//Add the vertex to the vertices vector
		vertices.push_back(vertex);
	}

	//Process indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		//Get the face with its indices
		aiFace face = mesh->mFaces[i];
		//Retrieve each index of the face and store it
		for (GLuint j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	//Process material
	if (mesh->mMaterialIndex >= 0)
	{
		//Get the material
		aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];

		//Retrieve the shininess property
		float shininess{32.0f};
		if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
		{
			//Set the shininess of the material
			material.shininess = static_cast<float>(shininess);
		}

		//Load the textures of the material
		std::vector<Texture> diffuseMaps = loadMaterialTextures(aiMat, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(aiMat, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<Texture> normalMaps = loadMaterialTextures(aiMat, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> heightMaps = loadMaterialTextures(aiMat, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}

	//Return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures, material);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
	//Vector to hold the textures
	std::vector<Texture> textures;

	//Iterate over each texture of the given type
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		//Get the texture path
		aiString str;
		mat->GetTexture(type, i, &str);

		//Check if texture was loaded before
		bool skip = false;
		for (const auto& loadedTexture : mLoadedTextures)
		{
			//If texture was loaded before, add it to the textures vector again
			if (std::strcmp(loadedTexture.path.data(), str.C_Str()) == 0)
			{
				textures.push_back(loadedTexture);
				skip = true;
				break;
			}
		}
		//If texture hasn't been loaded yet, load it
		if (!skip)
		{
			//Declare a texture object
			Texture texture{};

			//Load the texture from file
			texture.id = TextureFromFile(str.C_Str());

			//Set the texture type and path
			texture.type = typeName;
			texture.path = str.C_Str();

			//Add the texture to the textures vector
			textures.push_back(texture);

			//Add the texture to the loaded textures vector to avoid loading duplicates
			mLoadedTextures.push_back(texture);
		}
	}

	return textures;
}

GLuint Model::TextureFromFile(const char* path)
{
	//Generate the full filepath
	std::string filename = std::string(path);
	filename = mDirectory + '/' + filename;

	//Generate a texture ID
	GLuint textureID{};
	glGenTextures(1, &textureID);

	//Load the texture image
	SDL_Surface* surface = IMG_Load(filename.c_str());
	if (surface == nullptr)
	{
		KJK_ERROR("Failed to load texture image: {0}", SDL_GetError());
	}
	else
	{
		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, textureID);

		//Convert the surface to a standard format
		SDL_Surface* formattedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);

		//Free the original surface
		SDL_DestroySurface(surface);

		//Check if the conversion was successful
		if (formattedSurface == nullptr)
		{
			KJK_ERROR("Failed to convert surface to standard format: {0}", SDL_GetError());
		}
		else
		{
			//Generate the texture using the loaded surface data
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, formattedSurface->w, formattedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formattedSurface->pixels);

			//Set the texture wrapping/filtering parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			//Generate mipmaps
			glGenerateMipmap(GL_TEXTURE_2D);

			//Free the formatted surface
			SDL_DestroySurface(formattedSurface);
		}
	}

	KJK_INFO("Loaded texture at path: {0}", filename.c_str());

	//Return the success flag
	return textureID;
}
