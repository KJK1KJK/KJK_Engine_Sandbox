#pragma once

#include "BaseModel.h"

//Cube model class derived from BaseModel
class CubeModel : public BaseModel
{
public:
	//Constructor and destructor
	CubeModel(const char* diffuseTexturePath, const char* specularTexturePath, bool is2d = true, std::vector<std::string> facePaths = std::vector<std::string>{});
	~CubeModel();

	//Disable copy semantics
	CubeModel(const CubeModel& other) = delete;
	CubeModel& operator=(const CubeModel& other) = delete;

	//Allow move semantics
	CubeModel(CubeModel&& other) noexcept;
	CubeModel& operator=(CubeModel&& other) noexcept;

	//Override Draw method
	void Draw(const Shader& shader, glm::mat4 model = glm::mat4(1.0f)) const override;

private:
	//Initialize vertices and indices
	void initializeBuffers() override;

	//Initialize the model's vertex and index data for a cube texture
	void setupCubeModel(std::vector<std::string> facePaths);

	//load cube map textures
	GLuint loadCubemap(std::vector<std::string> faces);

private:
	GLuint mCubemapID;
};