#pragma once

#include "Shader.h"

struct Vertex
{
	glm::vec3 position; //Vertex position
	glm::vec3 normal; //Vertex normal
	glm::vec2 texCoords; //Vertex texture coordinates
	glm::vec3 tangent; //Vertex tangent
	glm::vec3 bitangent; //Vertex bitangent

	GLint m_BoneIDs[4]; //IDs of bones affecting this vertex
	GLfloat m_Weights[4]; //Weights of bones affecting this vertex
};

struct Texture
{
	GLuint id; //Texture ID
	std::string type; //Texture type (diffuse, specular, etc.)
	std::string path; //File path of the texture
};

class Mesh
{
public:
	//Mesh data
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	//Constructor
	Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures);

	//Render the mesh
	void Draw(const Shader& shader) const;

	//VAO getter
	inline GLuint GetVAO() const { return mVAO; }
private:
	//Render data
	GLuint mVAO, mVBO, mEBO;

	//Initializes all the buffer objects/arrays
	void setupMesh();
};