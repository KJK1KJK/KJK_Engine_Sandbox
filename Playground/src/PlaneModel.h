#pragma once

#include "BaseModel.h"

//Plane model class derived from BaseModel
class PlaneModel : public BaseModel
{
public:
	//Constructor and destructor
	PlaneModel(const char* diffuseTexturePath, const char* specularTexturePath);
	~PlaneModel();

	//Disable copy semantics
	PlaneModel(const PlaneModel& other) = delete;
	PlaneModel& operator=(const PlaneModel& other) = delete;

	//Allow move semantics
	PlaneModel(PlaneModel&& other) noexcept;
	PlaneModel& operator=(PlaneModel&& other) noexcept;

private:
	//Initialize vertices and indices
	void initializeBuffers() override;
};