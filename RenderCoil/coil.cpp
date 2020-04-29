#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
#include <string>
#include <ctime>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "sphere.h"
#include "light.h"
#include "materialPhong.h"
#include "materialCookTorrance.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
bool saveScreenshot(std::string filename, int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
double lastX = SCR_WIDTH / 2.0;
double lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// sphere initialzied as radius, sectors, stacks
Sphere sphere(1.0f, 36, 18);

// light
// Light light(glm::vec3(0.8f, 1.5f, 2.3f), 15.0f, glm::vec3(1.0f, 1.0f, 1.0f), LIGHT_POINT);

// Phong Material
// color, specular power, glossiness
// MaterialPhong phong(glm::vec3(0.7f, 1.4f, 2.5f), 1.0f, 5.0f);      
// color, specular power, roughness, fresnel
MaterialCookTorrance cookTorrance(glm::vec3(0.3f, 0.2f, 0.7f), 0.1f, glm::vec3(0.7f, 0.7f, 0.65f));

// timing
double deltaTime = 0.0;	// time between current frame and last frame
double lastFrame = 0.0;

int main()
{
    srand((unsigned int)time(0));
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Renderer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("./shader_code/shader.vert", "./shader_code/shader.frag");

    unsigned int VBO, VAO, IBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                (unsigned int)sphere.verticesBatch.size() * sizeof(float), 
                sphere.verticesBatch.data(), 
                GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                (unsigned int)sphere.indices.size() * sizeof(unsigned int), 
                sphere.indices.data(),
                GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(3 * sizeof(float)));
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(6 * sizeof(float)));

    // load and create a texture 
    // -------------------------
    // Texture texture1("resources/textures/container.jpg", GL_TEXTURE_2D);
    // Texture texture2("resources/textures/awesomeface.png", GL_TEXTURE_2D);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    // ourShader.use();
    // ourShader.setInt("texture1", 0);
    // ourShader.setInt("texture2", 1);


    // render loop
    // -----------
    // while (!glfwWindowShouldClose(window))
    for (float i = 0.0; i < 10.0; i += 1.0)
    {
    for (float j = 2.0; j < 1024.0; j *= 2.0)
    {
    for (int k = 0; k < 40; ++k)
    {
        float _r = (float)(rand() % 270 + 30) / 100.0f;
        float _g = (float)(rand() % 270 + 30) / 100.0f;
        float _b = (float)(rand() % 270 + 30) / 100.0f;
        glm::vec3 clr = glm::vec3(_r, _g, _b);
    /*for (int l = 0; l < 40; ++l)
    {*/
        float theta = (float)(rand() % 157 + 75) / 100.0f;
        float phi = (float)(rand() % 157 + 75) / 100.0f;
        float _x = sinf(theta) * cosf(phi) * 2.0f;
        float _y = cosf(theta) * 2.0f;
        float _z = sinf(theta) * sinf(phi) * 2.0f;
        //Light light(glm::vec3(_x, _y, _z), 15.0f, glm::vec3(1.0f, 1.0f, 1.0f), LIGHT_POINT);

        Light light(glm::vec3(0.8f, 1.5f, 2.3f), 15.0f, glm::vec3(1.0f, 1.0f, 1.0f), LIGHT_POINT);
        MaterialPhong phong(clr, i, j);

        //MaterialPhong phong(glm::vec3(0.7f, 1.4f, 2.5f), i, j);

        // per-frame time logic
        // --------------------
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        // bind textures on corresponding texture units
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture1.GetID());
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, texture2.GetID());

        // activate shader
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        ourShader.setMat4("model", model);

        // Rendering Model
        ourShader.setInt("modelType", MODEL_PHONG);

        // light
        ourShader.setVec3("lightPos", light.getPosition());
        ourShader.setVec3("lightColor", light.getColor());
        ourShader.setFloat("lightIntensity", light.getIntensity());
        ourShader.setInt("lightType", light.getType());

        // camera uniform variable
        ourShader.setVec3("viewPos", camera.getPosition());

        // material uniform variables
        ourShader.setVec3("phongColor", phong.getColor());
        ourShader.setFloat("phongGlossiness", phong.getGlossiness());
        ourShader.setFloat("phongSpecularPower", phong.getSpecularPower());
        ourShader.setVec3("cookColor", cookTorrance.getColor());
        ourShader.setVec3("cookFresnel", cookTorrance.getFresnel());
        ourShader.setFloat("cookRoughness", cookTorrance.getRoughness());

        // render boxes
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 
                    (unsigned int)sphere.indices.size(),
                    GL_UNSIGNED_INT,
                    (void*)0);

        std::string fn = "img/phong/phong_" + std::to_string((int)(i)) + "_" + std::to_string((int)j) + "_" + std::to_string(k)/* + "_" + std::to_string(l)*/ + ".jpg";
        saveScreenshot(fn, 80, 60);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    }
    //}
    }
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        saveScreenshot("screen01.jpg", 80, 60);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement((float)xoffset, (float)yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}

bool saveScreenshot(std::string filename, int width, int height)
{
    // row alignment
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    int nScrSize = SCR_WIDTH * SCR_HEIGHT * 4;
    int nSize = width * height * 4;
    unsigned char* dataBuffer = (unsigned char*)malloc(nScrSize * sizeof(unsigned char));
    unsigned char* resizedBuffer = (unsigned char*)malloc(nSize * sizeof(unsigned char));
    if (!dataBuffer) {
        std::cout << "saveScreenshot() :: buffer allocation error." << std::endl;   
        return false;
    }

    // fetch image from the backbuffer
    glReadPixels((GLint)0, (GLint)0, (GLint)SCR_WIDTH, (GLint)SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);

    stbir_resize(dataBuffer, SCR_WIDTH, SCR_HEIGHT, 0, resizedBuffer, width, height, 0,
                STBIR_TYPE_UINT8, 4, STBIR_ALPHA_CHANNEL_NONE, 0,
                STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT,
                STBIR_COLORSPACE_SRGB, nullptr);
    stbi_flip_vertically_on_write(true);
    stbi_write_jpg(filename.c_str(), width, height, 4, resizedBuffer, 100);

    free(dataBuffer);
    free(resizedBuffer);

    // std::cout << "saving screenshot(" << filename << ")\n";
    return true;
}