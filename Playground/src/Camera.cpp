#include "Camera.h"

Camera::Camera(const glm::vec3& startPosition)
	: position(startPosition)
{
	//Set the initial direction to look at the origin
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
	direction = glm::normalize(target - position);

	//Set the initial right vector
	right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));

	//Set the initial up vector
	up = glm::normalize(glm::cross(right, direction));
}

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 upVector)
	: position(position), direction(glm::normalize(target - position)), up(glm::normalize(upVector))
{
	//Calculate the right vector
	right = glm::normalize(glm::cross(direction, up));
}

Camera::Camera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ)
	: position(glm::vec3(posX, posY, posZ)),
	  direction(glm::normalize(glm::vec3(targetX, targetY, targetZ) - position)),
	  up(glm::normalize(glm::vec3(upX, upY, upZ)))
{
	//Calculate the right vector
	right = glm::normalize(glm::cross(direction, up));
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position, position + direction, up);
}

void Camera::HandleInput(const SDL_Event& e, float deltaTime, bool isMouseCaptured, const bool* keyboardState)
{
	//Handle keyboard input for camera movement
	if(e.type == SDL_EVENT_KEY_DOWN)
	{

	}
	//Handle mouse movement for camera orientation
	else if (isMouseCaptured && e.type == SDL_EVENT_MOUSE_MOTION)
	{
		//Get mouse movement deltas
		float xoffset = static_cast<float>(e.motion.xrel);
		float yoffset = static_cast<float>(e.motion.yrel);

		//Add the sensitivity factor
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		//Convert the direction vector to yaw and pitch angles
		float yaw = glm::degrees(atan2(direction.z, direction.x));
		float pitch = glm::degrees(asin(direction.y));

		//Update yaw and pitch based on mouse movement
		yaw += xoffset;
		pitch -= yoffset;

		//Constrain the pitch angle to prevent flipping
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		//Convert yaw and pitch back to a direction vector
		glm::vec3 newDirection{};
		newDirection.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		newDirection.y = sin(glm::radians(pitch));
		newDirection.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		direction = glm::normalize(newDirection);

		//Recalculate the right and up vectors
		right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
		up = glm::normalize(glm::cross(right, direction));
	}
	//Handle mouse wheel for zooming (FOV adjustment)
	else if (e.type == SDL_EVENT_MOUSE_WHEEL)
	{
		//Get the scroll amount
		float scroll = static_cast<float>(e.wheel.y);

		//Adjust the FoV based on scroll input
		fov -= scroll * scrollSensitivity;
		if (fov < 1.0f)
			fov = 1.0f;
		if (fov > 179.0f)
			fov = 179.0f;
	}

	//Handle continuous keyboard state for smoother movement
	if (keyboardState)
	{
		if (keyboardState[SDL_SCANCODE_W])
		{
			position += direction * speed * deltaTime;
		}
		if (keyboardState[SDL_SCANCODE_S])
		{
			position -= direction * speed * deltaTime;
		}
		if (keyboardState[SDL_SCANCODE_A])
		{
			position -= right * speed * deltaTime;
		}
		if (keyboardState[SDL_SCANCODE_D])
		{
			position += right * speed * deltaTime;
		}
	}
}
