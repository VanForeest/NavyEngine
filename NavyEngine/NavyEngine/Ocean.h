#pragma once

#include <GL/glew.h>
#include <glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


struct OceanParameters{
    alignas(8)  float windDirection[2]; // vec2 ocupa 8 bytes, pero el siguiente float se alineará según reglas std140
    alignas(4)  float windSpeed;        // float (4 bytes)
    alignas(4)  float fetch;            // float (4 bytes)
    alignas(4)  float gravity;          // float (4 bytes)
    alignas(4)  float gamma;            // float (4 bytes)
    alignas(4)  float L;                // float (4 bytes)
    alignas(4)  float AmplitudScale;    // float (4 bytes)    
    alignas(4)  int   N;                // int (4 bytes)

};

class Ocean{
public:

    OceanParameters Pams;
    GLuint UBO;

    Ocean(){
        this->Pams.windDirection[0] = 1.0f; //Valor en X
        this->Pams.windDirection[1] = 0.0f; //Valor en Z
        this->Pams.windSpeed = 11.5f; //U_10 11.5
        this->Pams.fetch = 1000000.0f; //(distancia en metros sobre la que sopla el viento)
        this->Pams.gravity = 9.81f;
        this->Pams.gamma = 3.3f; //Original 3.3
        this->Pams.L = 1000.0f; //Escala espacial del oceano
        this->Pams.AmplitudScale = 0.001; //Escala de amplitud
        this->Pams.N = 256; //Tiene que coincidir con la resolucion del mar y las texturas


        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);

        // Reservamos el espacio en la GPU
        glBufferData(GL_UNIFORM_BUFFER, sizeof(OceanParameters), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Vinculamos el búfer al punto de enlace (binding = 0)
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
    }

    void SetParameters(glm::vec2 wDir, float wSpeed, float fetch, float gravity, float gamma, float L, float AmScale, int N){
        this->Pams.windDirection[0] = wDir.x; //Valor en X
        this->Pams.windDirection[1] = wDir.y; //Valor en Z
        this->Pams.windSpeed = wSpeed; //U_10
        this->Pams.fetch = fetch; //Distancia en metros
        this->Pams.gravity = gravity;
        this->Pams.gamma = gamma;
        this->Pams.L = L; //Escala espacial del oceano
        this->Pams.AmplitudScale = AmScale;
        this->Pams.N = N; //Tiene que coincidir con la resolucion del mar y las texturas
    
    }

    void SendDataToComputeShader(){
     
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OceanParameters), &this->Pams);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
};
