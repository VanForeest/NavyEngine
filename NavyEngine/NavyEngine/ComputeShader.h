#pragma once

#include <GL/glew.h>
#include <glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


class ComputeShader{
public: 

	GLuint ID;

	ComputeShader(const char* ComputeFilePath){

		std::string ComputeCode;
		std::ifstream CshaderFile;

		CshaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {

			CshaderFile.open(ComputeFilePath);
			
			std::stringstream CshaderStream;
			CshaderStream << CshaderFile.rdbuf();
			
			CshaderFile.close();
		
			ComputeCode = CshaderStream.str();

		}
		catch (std::ifstream::failure& e) {
			std::cout << "ERROR::COMPUTE_SHADER::EL_ARCHIVO_NO_FUE_LEIDO_CORRECTAMENTE" << e.what() << std::endl;
		}

        const char* CshaderCode = ComputeCode.c_str();

        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader, 1, &CshaderCode, nullptr);
        glCompileShader(shader);

        // Verificar errores de compilación
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "ERROR::COMPUTE_SHADER::COMPILING\n" << infoLog << std::endl;
        }

        ID = glCreateProgram();
        glAttachShader(ID, shader);
        glLinkProgram(ID);

        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(ID, 512, nullptr, infoLog);
            std::cerr << "ERROR::LINKING_PROGRAM\n" << infoLog << std::endl;
        }

        glDeleteShader(shader);
		
	}

    void use(){
    
        glUseProgram(ID);
    }

	void setBool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	void setInt(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setVec2(const std::string& name, const glm::vec2& value) const {
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const {
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}

	void setVec3(const std::string& name, const glm::vec3& value) const {
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}

	void setVec4(const std::string& name, const glm::vec4& value) const {
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w) {
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}

	void setUint(const std::string& name, GLuint value){
		glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
	}

};