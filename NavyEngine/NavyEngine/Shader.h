#pragma once

#include <GL/glew.h>
#include <glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


class Shader {

public:
	GLuint ID;

	Shader(const char* VertexPath, const char* FragmentPath, const char* GeometryPath = nullptr){
		
		std::string VertexCode, FragCode, GeoCode;
		std::ifstream VshaderFile, FshaderFile, GshaderFile;

		VshaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		FshaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		GshaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try{

			VshaderFile.open(VertexPath);
			FshaderFile.open(FragmentPath);

			std::stringstream VshaderStream, FshaderStream;
			VshaderStream << VshaderFile.rdbuf();
			FshaderStream << FshaderFile.rdbuf();

			VshaderFile.close();
			FshaderFile.close();

			VertexCode = VshaderStream.str();
			FragCode = FshaderStream.str();

			if(GeometryPath != nullptr){
				GshaderFile.open(GeometryPath);
				std::stringstream GshaderStream;
				GshaderStream << GshaderFile.rdbuf();
				GshaderFile.close();
				GeoCode = GshaderStream.str();
			}
		}catch(std::ifstream::failure& e){
			std::cout << "ERROR::SHADER::EL_ARCHIVO_NO_FUE_LEIDO_CORRECTAMENTE" << e.what() << std::endl;
		}

		const char* VshaderCode = VertexCode.c_str();
		const char* FshaderCode = FragCode.c_str();

		GLuint Vshader, Fshader, Gshader;

		Vshader = glCreateShader(GL_VERTEX_SHADER);
		Fshader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(Vshader, 1, &VshaderCode, NULL);
		glShaderSource(Fshader, 1, &FshaderCode, NULL);

		glCompileShader(Vshader);
		checkCompileErrors(Vshader, "VERTEX");
		glCompileShader(Fshader);
		checkCompileErrors(Fshader, "FRAGMENT");

		if(GeometryPath != nullptr){
			const char* GshaderCode = GeoCode.c_str();	
			Gshader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(Gshader, 1, &GshaderCode, NULL);
			glCompileShader(Gshader);
			checkCompileErrors(Gshader, "GEOMETRY");
		}

		ID = glCreateProgram();
		glAttachShader(ID, Vshader);
		glAttachShader(ID, Fshader);

		if (GeometryPath != nullptr)
			glAttachShader(ID, Gshader);

		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");

		glDeleteShader(Vshader);
		glDeleteShader(Fshader);

		if (GeometryPath != nullptr)
			glDeleteShader(Gshader);
	}

	void use(){
		glUseProgram(ID);
	}

	void setBool(const std::string& name, bool value) const{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	
	void setInt(const std::string& name, int value) const{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	
	void setFloat(const std::string& name, float value) const{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	
	void setVec2(const std::string& name, const glm::vec2& value) const{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	
	void setVec3(const std::string& name, const glm::vec3& value) const{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	
	void setVec4(const std::string& name, const glm::vec4& value) const{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w){
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	
	void setMat2(const std::string& name, const glm::mat2& mat) const{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	
	void setMat3(const std::string& name, const glm::mat3& mat) const{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	
	void setMat4(const std::string& name, const glm::mat4& mat) const{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}


private:

	void checkCompileErrors(GLuint shader, std::string type){

		GLint success;
		GLchar infoLog[1024];

		if (type != "PROGRAM"){
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if(!success){

				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n" << std::endl;
			}
		}
		else{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success){
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n" << std::endl;
			}
		}
	}

};
