#pragma once

#include "BaseModel.h"

//Cube model class derived from BaseModel
class CubeModel : public BaseModel
{
public:
	//Constructor and destructor
	CubeModel(const char* diffuseTexturePath, const char* specularTexturePath);
	~CubeModel();

	//Disable copy semantics
	CubeModel(const CubeModel& other) = delete;
	CubeModel& operator=(const CubeModel& other) = delete;

	//Allow move semantics
	CubeModel(CubeModel&& other) noexcept;
	CubeModel& operator=(CubeModel&& other) noexcept;

private:
	//Initialize vertices and indices
	void initializeBuffers() override;
};