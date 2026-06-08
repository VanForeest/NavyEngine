#pragma once

#include <glm.hpp>

#include "Shader.h"
#include <string>

class PointLight{

public:

	glm::vec3 Color;
	glm::vec3 Position;
	float Range;

	PointLight(){
		this->Color = glm::vec3(1.0f);
		this->Position = glm::vec3(0.0f);
		this->Range = 3.0f;
	}


	PointLight(glm::vec3 color, glm::vec3 Pos, float range){
		this->Color = color;
		this->Position = Pos;
		this->Range = range;
	}

	void SendDataToShader(Shader& shader, const std::string& name) {
		shader.setVec3((name + ".Color"), Color);
		shader.setVec3((name + ".Position"), Position);
		shader.setFloat((name + ".Range"), Range);
	}

	void SendDataIndexToShader(Shader& shader, const std::string& name, int index) {

		std::string finalname = name + "[" + std::to_string(index) + "]";

		shader.setVec3((finalname + ".Color"), Color);
		shader.setVec3((finalname + ".Position"), Position);
		shader.setFloat((finalname + ".Range"), Range);
	}

};
