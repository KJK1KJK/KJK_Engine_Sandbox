#pragma once

class Camera
{
public:
	//Position in world space
	glm::vec3 position;
	//Direction the camera is looking
	glm::vec3 direction;
	//Right vector of the camera
	glm::vec3 right;
	//Up vector of the camera
	glm::vec3 up;

	//Camera speed
	const float speed{ 1.0f };
	//Mouse sensitivity
	const float sensitivity{ 0.1f };
	//Scroll sensitivity
	const float scrollSensitivity{ 2.0f };
	//Field of View
	float fov{ 85.0f };

	//Constructor that sets the starting position and looks at the origin
	Camera(const glm::vec3& startPosition);

	//Construct with vector parameters
	Camera(glm::vec3 position, glm::vec3 target, glm::vec3 upVector);

	//Construct with scalar parameters
	Camera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ);

	//Returns the view matrix
	glm::mat4 GetViewMatrix() const;

	//Handle user input
	void HandleInput(const SDL_Event& e, float deltaTime, bool isMouseCaptured, const bool* keyboardState);
};