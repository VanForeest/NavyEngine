#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <chrono>

#include "Utilities.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "Camera.h"
#include "Model.h"
#include "Ocean.h"
#include "DirectLight.h"
#include "PointLight.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void InitWindow();

// settings
GLFWwindow* window;
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float SimTime = 0.0f;
float SeaSpeed = 1.0f;

int main()
{
    InitWindow();

    camera.MoveSpeed = 20.0f;
    auto LastTime = std::chrono::high_resolution_clock::now();

    // build and compile shaders
    Shader FinalOcean("FinalOcean.vs", "NewFinalOcean.fs");
    FinalOcean.use();
    FinalOcean.setVec3("u_WaterColor", glm::vec3(0.01, 0.05, 0.09));
    //Color1 0.02, 0.08, 0.12
    //Color2 0.01, 0.05, 0.1
    //Color3 0.01, 0.05, 0.09
    FinalOcean.setVec3("u_FoamColor", glm::vec3(0.9, 0.9, 0.9));
    FinalOcean.setVec3("u_FF0", glm::vec3(0.02f));
    FinalOcean.setFloat("BaseRo", 0.05); //0.05
    FinalOcean.setFloat("FoamRo", 0.9);
    FinalOcean.setFloat("u_metallic", 0.0);
    FinalOcean.setFloat("u_ao", 1.0);

    Shader backgroundShader("SkyBox.vs", "SkyBox.fs");
    backgroundShader.use();
    backgroundShader.setInt("SkyBoxMap", 0);

    Shader TestShader("TestTexture.vs", "TestTexture.fs");

    ComputeShader GaussianShader("GaussianNoise.glsl");
    ComputeShader JONSWAPshader("JONSWAP.glsl");
    ComputeShader TimedJONSWAPshader("TimedJONSWAP.glsl");
    ComputeShader IFFTshader("IFFT.glsl");
    ComputeShader Permutationshader("Permutation.glsl");
    ComputeShader CombineTexshader("CombineTexture.glsl");
    ComputeShader OceanNormalFoamshader("OceanNormal_Foam.glsl");

    
  
    
    //3D model
    //Model masterreyModel("Modelos3D/masterrey.obj");
    Model OceanPlane("Modelos3D/OceanPlane256.obj");
    
    DirectLight Dirlight(glm::vec3(15.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    //Get IBL maps from HDRi file
    const std::string HDRfile = "HDR/LowSky.hdr";
    GLuint envCubemap, irradianceMap, prefilterMap, brdfLUTTexture;
    GenIBLmapsFromHDR(HDRfile, envCubemap, irradianceMap, prefilterMap, brdfLUTTexture);

    //Regresamos al viewport a su resolucion original , Limpieza necesaria despues de recolectar los IBL maps
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);


    //Otras instancias importantes
    float NormalSpace = 1.0f;
    float J_Umbral = 0.9; //Valor 1: 0.8 ; Valor 2: 0.85
    SeaSpeed = 0.8f;
    Ocean Oceano1;
    Oceano1.SetParameters(glm::vec2(0.1f, 0.0f), 10.5f, 1000000.0f, 9.81f, 2.0f, 1200.0f, 0.004, 256); //normal
    //Oceano1.SetParameters(glm::vec2(0.707f, 0.707f), 25.0f, 300000.0f, 9.81f, 1.5f, 2000.0f, 0.0015, 512); //Tormenta

    //Generar Texturas base 
    //Para Cualquiera menos NormalMap y FoamMap:GL_NEAREST
    //Para NormalMap y FoamMap: GL_LINEAR
    GLuint NoiseTexture = GenVoidTexture(Oceano1.Pams.N, GL_RGBA32F, GL_NEAREST);
    GLuint JonTexture = GenVoidTexture(Oceano1.Pams.N, GL_RGBA32F, GL_NEAREST);
    GLuint ButterflyTexture = GenButterflyTexture(Oceano1.Pams.N);

    GLuint DxTexture = GenVoidTexture(Oceano1.Pams.N, GL_RG32F, GL_NEAREST); //Movimiento en X
    GLuint DyTexture = GenVoidTexture(Oceano1.Pams.N, GL_RG32F, GL_NEAREST); //Movimiento en Y
    GLuint DzTexture = GenVoidTexture(Oceano1.Pams.N, GL_RG32F, GL_NEAREST); //Movimiento en Z
    GLuint SlopeX = GenVoidTexture(Oceano1.Pams.N, GL_RG32F, GL_NEAREST); //Contiene los valores de Dy / Dx
    GLuint SlopeZ = GenVoidTexture(Oceano1.Pams.N, GL_RG32F, GL_NEAREST); //Contiene los valores de Dy / Dz

    GLuint PingPongTexture = GenVoidTexture(Oceano1.Pams.N, GL_RG32F, GL_NEAREST);
    GLuint MoveTexture = GenVoidTexture(Oceano1.Pams.N, GL_RGBA32F, GL_NEAREST);
    GLuint NormalMap = GenVoidTexture(Oceano1.Pams.N, GL_RGBA32F, GL_LINEAR);
    GLuint FoamMap = GenVoidTexture(Oceano1.Pams.N, GL_R32F, GL_LINEAR);


    //Hacemos una nueva textura GaussianNoise
    GenGaussianNoiseTexture(Oceano1.Pams.N, NoiseTexture, GaussianShader);

    //Calculamos el estado inicial del espectro JONSWAP
    GenJONSWAPTexture(Oceano1.Pams.N, NoiseTexture, JonTexture, JONSWAPshader, Oceano1);

    //Valores estaticos
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 250.0f);

    FinalOcean.use();
    FinalOcean.setMat4("projection", projection);

    backgroundShader.use();
    backgroundShader.setMat4("projection", projection);
    


    //RENDER LOOP
    while (!glfwWindowShouldClose(window)){

        // Logica del tiempo per-frame
        auto CurrentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(CurrentTime - LastTime).count();
        LastTime = CurrentTime;

        // Evitar saltos gigantes si se congela un milisegundo
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        SimTime += deltaTime * SeaSpeed;

        //input
        processInput(window);

        //Texturas per-frame
        
        //NOTA IMPORTANTISIMA
        //Te hablo a vos masterrey del futuro o cualquiera que este traveseando el codigo
        //si queres probar las texturas JonTexture, DnTexture, Slopes (Modo view1)
        //si queres probar las texturas de MovementTextureN y Normalmap (Modo view2)
     
        //Modo view1: Desactivar el amplitudScale, Activar  shift_coord en JONSWAP.glsl
        // Activar FragColor = NewTexture; en TestTexture.fs
        
        //Modo view2: Activar amplitudScale, Desactivar shift_coord en JONSWAP.glsl
        //Activar FragColor = vec4(vec3(NewTexture.r), 1.0) para MovementTextureN
        // Activar FragColor = NewTexture; para NormalMap o MovementMap
        
        //Modo actual: view2

        //NOTA IMPORTANTISIMA 2
        //La simulacion del mar solo funciona si se esta en modo view2, esto porque los calculos requeridos
        //para la correcta simulacion del mar se dan en dicho patron de vista

        //Movimiento X, Y, Z, dominio frecuencia / TimedJONSWAP
        GenTimedJONSWAPTexture(Oceano1.Pams.N, JonTexture, Oceano1, TimedJONSWAPshader, SimTime, DxTexture, DyTexture, DzTexture, SlopeX, SlopeZ);

        //Texturas de Movimiento X, Y, Z, Dominio del Tiempo / IFFT
        ApplyIFFT(Oceano1.Pams.N, IFFTshader, Permutationshader, ButterflyTexture, DxTexture, PingPongTexture);
        ApplyIFFT(Oceano1.Pams.N, IFFTshader, Permutationshader, ButterflyTexture, DyTexture, PingPongTexture);
        ApplyIFFT(Oceano1.Pams.N, IFFTshader, Permutationshader, ButterflyTexture, DzTexture, PingPongTexture);
        ApplyIFFT(Oceano1.Pams.N, IFFTshader, Permutationshader, ButterflyTexture, SlopeX, PingPongTexture);
        ApplyIFFT(Oceano1.Pams.N, IFFTshader, Permutationshader, ButterflyTexture, SlopeZ, PingPongTexture);

        //Combinar Texturas de movimiento
        CombineMovementTextures(Oceano1.Pams.N, CombineTexshader, DxTexture, DyTexture, DzTexture, MoveTexture);

        //Generar normal map y foam map
        GenNormal_FoamTextures(Oceano1.Pams.N, OceanNormalFoamshader, NormalSpace, J_Umbral, MoveTexture, SlopeX, SlopeZ, NormalMap, FoamMap);



        //Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        

        //Renderizar el mar
        FinalOcean.use();
        FinalOcean.setMat4("view", view);
        FinalOcean.setVec3("camPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, MoveTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, NormalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, FoamMap);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        Dirlight.SendDataToShader(FinalOcean, "dirlight");

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

        FinalOcean.setMat4("model", model);
        FinalOcean.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));

        OceanPlane.Draw(FinalOcean);


        //render skybox
        backgroundShader.use();
        backgroundShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
        //glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
        renderCube();


        //RECORDAR PONER AQUI EL CONTENIDO DE InsideMain.txt
        //Zona de prueba para las texturas generadas por los Compute Shader
        TestShader.use();
        TestShader.setInt("Texture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, MoveTexture);
        //renderQuad();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}




void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.HandleKeyboard(FRONT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.HandleKeyboard(BACK, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.HandleKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.HandleKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.HandleKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.HandleKeyboard(DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.HandleMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.HandleMouseScroll(static_cast<float>(yoffset));
}

void InitWindow(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "NavyEngine", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL){
        std::cout << "Error al crear GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Error al iniciar Glew" << std::endl;
    }

    //configuramos los global opengl state
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LEQUAL); //Activado para el skybox
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //Activado para el multiple mip map sampling para los prefilters

    std::cout << "Version de OpenGL en uso: " << glGetString(GL_VERSION) << std::endl;
}

