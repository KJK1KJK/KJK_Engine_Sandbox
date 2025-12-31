#include "BaseModel.h"

#include <KJK_Engine/Core/Logger.h>

BaseModel::BaseModel(const char* diffuseTexturePath, const char* specularTexturePath)
	:position(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), rotation(0.0f, 0.0f, 0.0f), mVAO(0), mVBO(0), mEBO(0), mDiffuseId(0), mSpecularId(0)
{
}

BaseModel::~BaseModel()
{
	//Delete the buffers/arrays
	if (mVAO != 0)
		glDeleteVertexArrays(1, &mVAO);
	if (mVBO != 0)
		glDeleteBuffers(1, &mVBO);
	if (mEBO != 0)
		glDeleteBuffers(1, &mEBO);
	//Delete textures
	if (mDiffuseId != 0)
		glDeleteTextures(1, &mDiffuseId);
	if (mSpecularId != 0)
		glDeleteTextures(1, &mSpecularId);
}

BaseModel::BaseModel(BaseModel&& other) noexcept
	: position(other.position), scale(other.scale), rotation(other.rotation), mVAO(other.mVAO), mVBO(other.mVBO), mEBO(other.mEBO), mDiffuseId(other.mDiffuseId), mSpecularId(other.mSpecularId), vertices(std::move(other.vertices)), indices(std::move(other.indices))
{
	//Invalidate other's resources
	other.mVAO = 0;
	other.mVBO = 0;
	other.mEBO = 0;
	other.mDiffuseId = 0;
	other.mSpecularId = 0;
}

BaseModel& BaseModel::operator=(BaseModel&& other) noexcept
{
	if (this != &other)
	{
		//Delete existing resources
		if (mVAO != 0)
			glDeleteVertexArrays(1, &mVAO);
		if (mVBO != 0)
			glDeleteBuffers(1, &mVBO);
		if (mEBO != 0)
			glDeleteBuffers(1, &mEBO);

		//Delete textures
		if (mDiffuseId != 0)
			glDeleteTextures(1, &mDiffuseId);
		if (mSpecularId != 0)
			glDeleteTextures(1, &mSpecularId);

		//Move data from other
		position = other.position;
		scale = other.scale;
		rotation = other.rotation;
		mVAO = other.mVAO;
		mVBO = other.mVBO;
		mEBO = other.mEBO;
		mDiffuseId = other.mDiffuseId;
		mSpecularId = other.mSpecularId;
		vertices = std::move(other.vertices);
		indices = std::move(other.indices);

		//Invalidate other's resources
		other.mVAO = 0;
		other.mVBO = 0;
		other.mEBO = 0;
		other.mDiffuseId = 0;
		other.mSpecularId = 0;
	}

	return *this;
}

void BaseModel::Draw(const Shader& shader, glm::mat4 model) const
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

	//Bind the VAO
	glBindVertexArray(mVAO);

	//Draw the model
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

	//Unbind the VAO
	glBindVertexArray(0);
}

void BaseModel::setBufferData(const std::vector<BaseVertex>& verts, const std::vector<GLuint>& inds)
{
	//Update vertices and indices
	vertices = verts;
	indices = inds;

	//Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	//Update VBO data
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(BaseVertex), &vertices[0], GL_STATIC_DRAW);

	//Bind the EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	//Update EBO data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
}

void BaseModel::setup(const char* diffuseTexturePath, const char* specularTexturePath)
{
	//Load the textures
	mDiffuseId = TextureFromFile(diffuseTexturePath);
	mSpecularId = TextureFromFile(specularTexturePath);

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

GLuint BaseModel::TextureFromFile(const char* path)
{
	//Generate a texture ID
	GLuint textureID{};
	glGenTextures(1, &textureID);

	//Load the texture image
	SDL_Surface* surface = IMG_Load(path);
	if (surface == nullptr)
	{
		KJK_ERROR("Failed to load texture image: {0}", SDL_GetError());
	}
	else
	{
		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, textureID);

		//Check if the surface has an alpha channel
		const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface->format);
		bool hasAlpha = (details && details->Amask != 0);

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

			//Get the texture wrap mode from the alpha channel presence
			GLenum wrapMode = hasAlpha ? GL_CLAMP_TO_EDGE : GL_REPEAT;

			//Set the texture wrapping/filtering parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			//Generate mipmaps
			glGenerateMipmap(GL_TEXTURE_2D);

			//Free the formatted surface
			SDL_DestroySurface(formattedSurface);
		}
	}

	KJK_INFO("Loaded texture at path: {0}", path);

	//Return the success flag
	return textureID;
}
