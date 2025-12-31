#pragma once

#include "Shader.h"

struct BaseVertex
{
	glm::vec3 position; //Vertex position
	glm::vec3 normal; //Vertex normal
	glm::vec2 texCoords; //Vertex texture coordinates
};

//Abstract base class for 3D models
class BaseModel
{
protected:
	//Transformation properties
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;

	//Texture scale
	float textureScale{ 1.0f };

	//Vertices and indices
	std::vector<BaseVertex> vertices;
	std::vector<GLuint> indices;
public:
	//Constructor and destructor
	BaseModel(const char* diffuseTexturePath, const char* specularTexturePath);
	virtual ~BaseModel();

	//Disable copy semantics
	BaseModel(const BaseModel& other) = delete;
	BaseModel& operator=(const BaseModel& other) = delete;

	//Allow move semantics
	BaseModel(BaseModel&& other) noexcept;
	BaseModel& operator=(BaseModel&& other) noexcept;

	//Draw the cube
	virtual void Draw(const Shader& shader, glm::mat4 model = glm::mat4(1.0f)) const;

	//Getters for transformation properties
	inline glm::vec3 getPosition() const { return position; }
	inline glm::vec3 getScale() const { return scale; }
	inline glm::vec3 getRotation() const { return rotation; }

	//Setters for transformation properties
	inline void setPosition(const glm::vec3& pos) { position = pos; }
	inline void setScale(const glm::vec3& scl) { scale = scl; }
	inline void setRotation(const glm::vec3& rot) { rotation = rot; }

	//Getter and setter for texture scale
	inline float getTextureScale() const { return textureScale; }
	inline void setTextureScale(float texScale) { textureScale = texScale; }

	//Getters for vertices and indices
	inline const std::vector<BaseVertex>& getVertices() const { return vertices; }
	inline const std::vector<GLuint>& getIndices() const { return indices; }

	//Setters for vertices and indices
	void setBufferData(const std::vector<BaseVertex>& verts, const std::vector<GLuint>& inds);
protected:
	//OpenGL object IDs
	GLuint mVAO, mVBO, mEBO;
	//Texture IDs
	GLuint mDiffuseId, mSpecularId;

	//Initializes all the buffer objects/arrays
	void setup(const char* diffuseTexturePath, const char* specularTexturePath);

	//Initialize vertices and indices
	virtual void initializeBuffers() = 0;

	//Load a texture from file
	GLuint TextureFromFile(const char* path);
};