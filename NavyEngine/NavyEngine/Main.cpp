#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Utilities.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
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
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float lastX = 1280.0f / 2.0;
float lastY = 720.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    InitWindow();

    camera.MoveSpeed = 15.0f;

    // build and compile shaders
    // -------------------------
    Shader pbrShader("PBR.vs", "PBR.fs");
    Shader TexturedpbrShader("WindPBR.vs", "TexturedPBR.fs");
    Shader backgroundShader("SkyBox.vs", "SkyBox.fs");
    Shader OceanShader("Ocean.vs","PBR.fs");

    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);
    pbrShader.setVec3("albedo", 0.01, 0.05, 0.1);
    pbrShader.setFloat("roughness", 0.03);
    pbrShader.setFloat("metallic", 0.0);
    pbrShader.setFloat("ao", 1.0f);
    pbrShader.setVec3("FF0", glm::vec3(0.02f));

    backgroundShader.use();
    backgroundShader.setInt("SkyBoxMap", 0);

    TexturedpbrShader.use();
    TexturedpbrShader.setInt("texture_BaseColor1", 0);
    TexturedpbrShader.setInt("texture_Normal1", 1);
    TexturedpbrShader.setInt("texture_ORM1", 2);
    TexturedpbrShader.setInt("irradianceMap", 3);
    TexturedpbrShader.setInt("prefilterMap", 4);
    TexturedpbrShader.setInt("brdfLUT", 5);

    OceanShader.use();
    OceanShader.setInt("irradianceMap", 0);
    OceanShader.setInt("prefilterMap", 1);
    OceanShader.setInt("brdfLUT", 2);
    OceanShader.setVec3("albedo", 0.02, 0.08, 0.12); //1) 0.01, 0.05, 0.1  2) 0.02, 0.08, 0.12 3) 0.01, 0.05, 0.09
    OceanShader.setFloat("roughness", 0.03); //0.03 //0.12
    OceanShader.setFloat("metallic", 0.0);
    OceanShader.setFloat("ao", 1.0f);
    OceanShader.setVec3("FF0", glm::vec3(0.02f));

    OceanShader.setFloat("Amplitude", 1.0);
    OceanShader.setFloat("wavelenght", 4.0);
    OceanShader.setFloat("speed", 2.0);
    

    //3D model
    Model masterreyModel("Modelos3D/masterrey.obj");
    Model OceanPlane("Modelos3D/OceanPlane3.obj");
    Model IslandModel("Modelos3D/Island_Final.glb");
    


    //Lights
    DirectLight Dirlight(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 3.0f));

    PointLight light1(glm::vec3(100.0f), glm::vec3(-10.0f, 10.0f, 10.0f), 10.0f);
    PointLight light2(glm::vec3(150.0f), glm::vec3(10.0f, 10.0f, 10.0f), 15.0f);
    PointLight light3(glm::vec3(200.0f), glm::vec3(-10.0f, -10.0f, 10.0f), 20.0f);
    PointLight light4(glm::vec3(250.0f), glm::vec3(10.0f, -10.0f, 10.0f), 25.0f);

    std::vector<PointLight> Lights{ light1, light2, light3, light4 };

    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;

    //Get IBL maps from HDRi file
    const std::string HDRfile = "HDR/LowSky.hdr";
    GLuint envCubemap, irradianceMap, prefilterMap, brdfLUTTexture;
    GenIBLmapsFromHDR(HDRfile, envCubemap, irradianceMap, prefilterMap, brdfLUTTexture);


    //Valores estaticos
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 150.0f);
    pbrShader.use();
    pbrShader.setMat4("projection", projection);
    backgroundShader.use();
    backgroundShader.setMat4("projection", projection);
    TexturedpbrShader.use();
    TexturedpbrShader.setMat4("projection", projection);
    OceanShader.use();
    OceanShader.setMat4("projection", projection);

    //Regresamos al viewport a su resolucion original
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    //RENDER LOOP
    while (!glfwWindowShouldClose(window)){

        // Logica del tiempo per-frame 
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //input
        processInput(window);

        //render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        glm::mat4 model = glm::mat4(1.0f);


        //SHADER DEL PBR CON TEXTURA
        TexturedpbrShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        TexturedpbrShader.setMat4("view", view);
        TexturedpbrShader.setVec3("camPos", camera.Position);
        TexturedpbrShader.setFloat("time", currentFrame);

        Dirlight.SendDataToShader(TexturedpbrShader, "dirlight");

        /*for (unsigned int i = 0; i < 4; ++i) {
            Lights[i].SendDataIndexToShader(TestpbrShader, "polight", i);
        }*/

        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        //Render masterrey
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -12.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        TexturedpbrShader.setMat4("model", model);
        TexturedpbrShader.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));

        masterreyModel.Draw(TexturedpbrShader);

        //Render Island
        model = glm::mat4(1.0f);
        // Desplazamos la isla detrás del masterrey model (en el eje Z negativo). 
        // Como masterrey está en Z = -12.0, colocar la isla en Z = -60.0 la pondrá justo a sus espaldas.
        model = glm::translate(model, glm::vec3(0.0f, 0.0f,60.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        TexturedpbrShader.setMat4("model", model);
        TexturedpbrShader.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));

        IslandModel.Draw(TexturedpbrShader);

        //USAMOS EL OCEAN SHADER
        OceanShader.use();
        OceanShader.setMat4("view", view);
        OceanShader.setVec3("camPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        //Render Ocean Plane
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(2.0f));
        OceanShader.setFloat("time", currentFrame);
        OceanShader.setMat4("model", model);
        OceanShader.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));

        OceanPlane.Draw(OceanShader);


        //USAMOR EL SHADER DEL PBR SIN TEXTURA
        /*pbrShader.use();
        pbrShader.setMat4("view", view);
        pbrShader.setVec3("camPos", camera.Position);

        Dirlight.SendDataToShader(pbrShader, "dirlight");

        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        //Render las columnas de esferas para la prueba de 
        for (int row = 0; row < nrRows; ++row)
        {
            pbrShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                pbrShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));
                pbrShader.setMat4("model", model);
                pbrShader.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                renderSphere();
            }
        }*/


        //render skybox
        backgroundShader.use();
        backgroundShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
        //glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
        renderCube();


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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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

    if (glewInit() != GLEW_OK) {
        std::cout << "Error al iniciar Glew" << std::endl;
    }

    //configuramos los global opengl state
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LEQUAL); //Activado para el skybox
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //Activado para el multiple mip map sampling para los prefilters
}

