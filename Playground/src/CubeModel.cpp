#include "CubeModel.h"

#include <KJK_Engine/Core/Logger.h>

CubeModel::CubeModel(const char* diffuseTexturePath, const char* specularTexturePath, bool is2d, std::vector<std::string> facePath)
	: BaseModel(diffuseTexturePath, specularTexturePath), mCubemapID(0)
{
	//Initialize the model
	if (is2d)
	{
		setup(diffuseTexturePath, specularTexturePath);

		KJK_INFO("Cube instantiated with diffuse texture: {}, specular texture: {}", diffuseTexturePath, specularTexturePath);
	}
	else
	{
		setupCubeModel(facePath);

		KJK_INFO("Cube instantiated with cubemap textures.");
	}
}

CubeModel::~CubeModel()
{
	//Delete the cubemap texture
	if (mCubemapID != 0)
	{
		glDeleteTextures(1, &mCubemapID);
	}
}

CubeModel::CubeModel(CubeModel&& other) noexcept
	: BaseModel(std::move(other)), mCubemapID(other.mCubemapID)
{
	//Invalidate other's mCubemapID
	other.mCubemapID = 0;
}

CubeModel& CubeModel::operator=(CubeModel&& other) noexcept
{
	//Use BaseModel's move assignment operator
	BaseModel::operator=(std::move(other));

	//Move mCubemapID
	mCubemapID = other.mCubemapID;

	//Invalidate other's mCubemapID
	other.mCubemapID = 0;

	return *this;
}

void CubeModel::Draw(const Shader& shader, glm::mat4 model) const
{
	//Use the shader program
	shader.Use();

	//Adjust the model matrix based on position, scale, and rotation
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);

	//Use the model matrix
	shader.SetMat4("model", model);

	//Set the texture scale uniform
	shader.SetFloat("textureScale", textureScale);

	//Check if using cubemap
	if (mCubemapID != 0)
	{
		//Bind the cubemap texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapID);

		//Set the cubemap sampler uniform
		shader.SetInt("skybox", 0);
	}
	else
	{
		//Bind the diffuse texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mDiffuseId);

		//Set the diffuse sampler uniform
		shader.SetInt("material.diffuse", 0);

		//Bind the specular texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mSpecularId);

		//Set the specular sampler uniform
		shader.SetInt("material.specular", 1);
	}

	//Bind the VAO
	glBindVertexArray(mVAO);

	//Draw the model
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

	//Unbind the VAO
	glBindVertexArray(0);
}

void CubeModel::initializeBuffers()
{
	//Define the cube's vertices
	vertices = {
		//Position		        //Normal              //Texture
		//Front
		{{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},
		//Back					  		 
		{{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
		//Left							 
		{{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
		//Right							 	    
		{{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
		//Top					  		 	    
		{{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
		//Bottom				  
		{{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}}
	};

	//Define the cube's indices
	indices = {
		//Front triangles
		0, 1, 2,
		2, 3, 0,
		//Back triangles
		4, 5, 6,
		6, 7, 4,
		//Left triangles
		8, 9, 10,
		10, 11, 8,
		//Right triangles
		12, 13, 14,
		14, 15, 12,
		//Top triangles
		16, 17, 18,
		18, 19, 16,
		//Bottom triangles
		20, 21, 22,
		22, 23, 20
	};
}

void CubeModel::setupCubeModel(std::vector<std::string> facePaths)
{
	//Load the 3d texture
	mCubemapID = loadCubemap(facePaths);

	//Initialize vertices and indices
	initializeBuffers();

	//Generate and bind VAO
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	//Generate and bind VBO
	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	//Generate and bind EBO
	glGenBuffers(1, &mEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);

	//Fill VBO with vertex data
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(BaseVertex), &vertices[0], GL_STATIC_DRAW);

	//Fill EBO with index data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	//Set the vertex attribute position pointer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BaseVertex), (void*)0);

	//Set the vertex attribute normal pointer
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BaseVertex), (void*)offsetof(BaseVertex, normal));

	//Set the vertex attribute texture coordinates pointer
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BaseVertex), (void*)offsetof(BaseVertex, texCoords));

	//Unbind the VAO
	glBindVertexArray(0);
}

GLuint CubeModel::loadCubemap(std::vector<std::string> faces)
{
	//Generate a texture ID
	GLuint textureID;
	glGenTextures(1, &textureID);

	for(GLuint i = 0; i < faces.size(); i++)
	{
		//Load the texture image
		SDL_Surface* surface = IMG_Load(faces[i].c_str());
		if (surface == nullptr)
		{
			KJK_ERROR("Failed to load texture image: {0}", SDL_GetError());
		}
		else
		{
			//Bind the texture
			glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

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
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, formattedSurface->w, formattedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formattedSurface->pixels);

				//Free the formatted surface
				SDL_DestroySurface(formattedSurface);
			}
		}
	}

	//Set the texture wrapping/filtering parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	//Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	return textureID;
}
