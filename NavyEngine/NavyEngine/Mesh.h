#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shader.h"
#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex{

	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TextureUV;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;

	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture{
	GLuint id;
	std::string type;
	std::string path;
};

class Mesh{
public:

	std::vector<Vertex> Vertices;
	std::vector<GLuint> Indices;
	std::vector<Texture> Texturas;
	GLuint VAO;

	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> texturas){

        this->Vertices = vertices;
        this->Indices = indices;
        this->Texturas = texturas;

        setupMesh();
	
	}

    void Draw(Shader& shader){
        
        GLuint basecolorNr = 1;
        GLuint normalNr = 1;       
        GLuint ORMNr = 1;

        std::string name;
        std::string number;

        for(int i = 0; i < Texturas.size(); i++){
            
            glActiveTexture(GL_TEXTURE0 + i);
            name = Texturas[i].type;

            if (name == "texture_BaseColor")
                number = std::to_string(basecolorNr++);
            else if (name == "texture_Normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_ORM")
                number = std::to_string(ORMNr++);

            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, Texturas[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(Indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:
    
    GLuint VBO, EBO;
    
    void setupMesh(){

        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vertex), &Vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);

       
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureUV));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};