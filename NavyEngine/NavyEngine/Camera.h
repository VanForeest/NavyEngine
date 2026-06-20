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

	// Limites del recorrido virtual (Bounding Box / Zona segura)
	float MinX, MaxX, MinZ, MaxZ;

	Camera(glm::vec3 Pos, glm::vec3 WUp) {

		Position = Pos;
		WorldUp = WUp;
		Front = glm::vec3(0.0f, 0.0f, -1.0f);

		MoveSpeed = 4.0f;
		Sensibility = 0.1f;
		Zoom = 65.0f;

		Yaw = -90.0f;
		Pitch = 0.0f;

		MaxZoom = 5.0f;
		MinZoom = 90.0f;

		// Limites de la orilla (caja de arena)
		// Centro aproximado: X=1.35, Z=23.32
		MinX = -40.0f;
		MaxX =  40.0f;
		MinZ =  15.2285f; // Evita que camines hacia el mar (Z < 15.2285)
		MaxZ =  26.9629f; // Evita que subas a la montaña (Z > 26.9629)

		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix() {
		return glm::lookAt(Position, Position + Front, Up);
	}

	void HandleKeyboard(Camera_Movement direction, float deltatime) {

		float velocity = MoveSpeed * deltatime;

		// Vectores proyectados en el plano XZ para caminar sin volar
		glm::vec3 frontXZ = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
		glm::vec3 rightXZ = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

		if (direction == FRONT)
			Position += frontXZ * velocity;
		if (direction == BACK)
			Position -= frontXZ * velocity;
		if (direction == RIGHT)
			Position += rightXZ * velocity;
		if (direction == LEFT)
			Position -= rightXZ * velocity;
		
		// Las teclas Q y E permiten subir/bajar verticalmente para depuracion
		if (direction == UP)
			Position += WorldUp * velocity;
		if (direction == DOWN)
			Position -= WorldUp * velocity;

		// --- APLICAR LIMITES DE LA ZONA SEGURA (COLLISION FAKE) ---
		if (Position.x < MinX) Position.x = MinX;
		if (Position.x > MaxX) Position.x = MaxX;
		if (Position.z < MinZ) Position.z = MinZ;
		if (Position.z > MaxZ) Position.z = MaxZ;

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

