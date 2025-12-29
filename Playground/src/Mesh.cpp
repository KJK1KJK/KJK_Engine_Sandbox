#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures, const Material& material)
	: vertices(vertices), indices(indices), textures(textures), material(material)
{
	setupMesh();
}

Mesh::~Mesh()
{
	//Delete the buffers/arrays
	if (mVAO != 0)
		glDeleteVertexArrays(1, &mVAO);
	if (mVBO != 0)
		glDeleteBuffers(1, &mVBO);
	if (mEBO != 0)
		glDeleteBuffers(1, &mEBO);

	//Delete textures
	for (const auto& texture : textures)
	{
		if (texture.id != 0)
			glDeleteTextures(1, &texture.id);
	}
}

Mesh::Mesh(Mesh&& other) noexcept
	: vertices(std::move(other.vertices)), indices(std::move(other.indices)), textures(std::move(other.textures)), material(other.material), mVAO(other.mVAO), mVBO(other.mVBO), mEBO(other.mEBO)
{
	//Invalidate other's resources
	other.mVAO = 0;
	other.mVBO = 0;
	other.mEBO = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if(this != &other)
	{
		//Delete existing resources
		if (mVAO != 0)
			glDeleteVertexArrays(1, &mVAO);
		if (mVBO != 0)
			glDeleteBuffers(1, &mVBO);
		if (mEBO != 0)
			glDeleteBuffers(1, &mEBO);

		//Delete textures
		for (const auto& texture : textures)
		{
			if (texture.id != 0)
				glDeleteTextures(1, &texture.id);
		}

		//Move data from other
		vertices = std::move(other.vertices);
		indices = std::move(other.indices);
		textures = std::move(other.textures);
		material = other.material;
		mVAO = other.mVAO;
		mVBO = other.mVBO;
		mEBO = other.mEBO;

		//Reset other
		other.mVAO = 0;
		other.mVBO = 0;
		other.mEBO = 0;
	}

	return *this;
}

void Mesh::Draw(const Shader& shader) const
{
	//Declare counters for each texture type
	GLuint diffuseNr = 1;
	GLuint specularNr = 1;
	GLuint normalNr = 1;
	GLuint heightNr = 1;

	//Iterate over all textures
	for (GLuint i = 0; i < textures.size(); i++)
	{
		//Activate proper texture unit before binding
		glActiveTexture(GL_TEXTURE0 + i);

		//Retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);
		else if (name == "texture_normal")
			number = std::to_string(normalNr++);
		else if (name == "texture_height")
			number = std::to_string(heightNr++);

		//Set the sampler to the correct texture unit
		shader.SetInt((name + number).c_str(), i);

		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	//Set the material shininess uniform
	shader.SetFloat("material.shininess", material.shininess);

	//Set the active texture back to default
	glActiveTexture(GL_TEXTURE0);

	//Bind the VAO
	glBindVertexArray(mVAO);
	
	//Draw the mesh
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

	//Unbind the VAO
	glBindVertexArray(0);
}

void Mesh::setupMesh()
{
	//Generate buffers/arrays
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	//Bind VAO
	glBindVertexArray(mVAO);

	//Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	//Fill VBO with vertex data
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	//Bind EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	//Fill EBO with index data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	//Set the vertex attribute position pointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	//Set the vertex attribute normal pointer
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);

	//Set the vertex attribute texture coordinate pointer
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);

	//Set the vertex attribute tangent pointer
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	glEnableVertexAttribArray(3);

	//Set the vertex attribute bitangent pointer
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
	glEnableVertexAttribArray(4);

	//Set the vertex attribute bone IDs pointer
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	glEnableVertexAttribArray(5);

	//Set the vertex attribute weights pointer
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
	glEnableVertexAttribArray(6);

	//Unbind VAO
	glBindVertexArray(0);
}
