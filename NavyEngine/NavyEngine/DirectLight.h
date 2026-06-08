#pragma once

#include <glm.hpp>

#include "Shader.h"
#include <string>

class DirectLight{

public:

	glm::vec3 Color;
	glm::vec3 Direction;

	DirectLight(){
		this->Color = glm::vec3(1.0f);
		this->Direction = glm::vec3(0.0f, -1.0f, 1.0f);
	}


	DirectLight(glm::vec3 color, glm::vec3 Dir){
		this->Color = color;
		this->Direction = Dir;
	}

	void SendDataToShader(Shader& shader, const std::string& name){		
		shader.setVec3((name + ".Color"), Color);
		shader.setVec3((name + ".Direction"), Direction);
	}

	void SendDataIndexToShader(Shader& shader, const std::string& name, int index){
	
		std::string finalname = name + "[" + std::to_string(index) + "]";

		shader.setVec3((finalname + ".Color"), Color);
		shader.setVec3((finalname + ".Direction"), Direction);
	}

};
