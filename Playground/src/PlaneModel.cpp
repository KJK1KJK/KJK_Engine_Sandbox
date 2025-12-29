#include "PlaneModel.h"

#include <KJK_Engine/Core/Logger.h>

PlaneModel::PlaneModel(const char* diffuseTexturePath, const char* specularTexturePath)
	: BaseModel(diffuseTexturePath, specularTexturePath)
{
	//Initialize the model
	setup(diffuseTexturePath, specularTexturePath);

	KJK_INFO("Cube instantiated with diffuse texture: {}, specular texture: {}", diffuseTexturePath, specularTexturePath);
}

PlaneModel::~PlaneModel()
{
}

PlaneModel::PlaneModel(PlaneModel&& other) noexcept
	: BaseModel(std::move(other))
{
}

PlaneModel& PlaneModel::operator=(PlaneModel&& other) noexcept
{
	//Use BaseModel's move assignment operator
	BaseModel::operator=(std::move(other));

	return *this;
}

void PlaneModel::initializeBuffers()
{
	//Define the plane's vertices
	vertices = {
		//Position		        //Normal              //Texture
		{{-0.5f, 0.0f, -0.5f}, { 0.0f, 1.0f,  0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, 0.0f, -0.5f}, { 0.0f, 1.0f,  0.0f}, {5.0f, 0.0f}},
		{{ 0.5f, 0.0f,  0.5f}, { 0.0f, 1.0f,  0.0f}, {5.0f, 5.0f}},
		{{-0.5f, 0.0f,  0.5f}, { 0.0f, 1.0f,  0.0f}, {0.0f, 5.0f}}
	};

	//Define the plane's indices
	indices = {
		0, 1, 2,
		0, 2, 3
	};
}
