#include "CubeModel.h"

#include <KJK_Engine/Core/Logger.h>

CubeModel::CubeModel(const char* diffuseTexturePath, const char* specularTexturePath)
	: BaseModel(diffuseTexturePath, specularTexturePath)
{
	//Initialize the model
	setup(diffuseTexturePath, specularTexturePath);

	KJK_INFO("Cube instantiated with diffuse texture: {}, specular texture: {}", diffuseTexturePath, specularTexturePath);
}

CubeModel::~CubeModel()
{
}

CubeModel::CubeModel(CubeModel&& other) noexcept
	: BaseModel(std::move(other))
{
}

CubeModel& CubeModel::operator=(CubeModel&& other) noexcept
{
	//Use BaseModel's move assignment operator
	BaseModel::operator=(std::move(other));

	return *this;
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
