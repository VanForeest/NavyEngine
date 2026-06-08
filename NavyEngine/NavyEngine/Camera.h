#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

enum Camera_Movement{
	FRONT, 
	BACK,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera {
public:

	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float MoveSpeed, Sensibility, Zoom;
	float Yaw, Pitch;

	float MaxZoom, MinZoom;

	Camera(glm::vec3 Pos, glm::vec3 WUp) {

		Position = Pos;
		WorldUp = WUp;
		Front = glm::vec3(0.0f, 0.0f, -1.0f);

		MoveSpeed = 5.0f;
		Sensibility = 0.1f;
		Zoom = 45.0f;

		Yaw = -90.0f;
		Pitch = 0.0f;

		MaxZoom = 5.0f;
		MinZoom = 90.0f;

		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix() {
		return glm::lookAt(Position, Position + Front, Up);
	}

	void HandleKeyboard(Camera_Movement direction, float deltatime) {

		float velocity = MoveSpeed * deltatime;

		if (direction == FRONT)
			Position += Front * velocity;
		if (direction == BACK)
			Position -= Front * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == UP)
			Position += Up * velocity;
		if (direction == DOWN)
			Position -= Up * velocity;

	}

	void HandleMouseMovement(float Xoffset, float Yoffset) {

		Xoffset *= Sensibility;
		Yoffset *= Sensibility;

		Yaw += Xoffset;
		Pitch += Yoffset;

		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;

		updateCameraVectors();
	}

	void HandleMouseScroll(float Yoffset){
		
		Zoom -= Yoffset;

		if (Zoom < MaxZoom)
			Zoom = MaxZoom;
		if (Zoom > MinZoom)
			Zoom = MinZoom;
		
	}


private:
	
	void updateCameraVectors(){
		
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}
};

