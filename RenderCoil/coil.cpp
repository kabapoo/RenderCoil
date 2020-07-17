#include "coil.h"

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// sphere initialzied as radius, sectors, stacks
Sphere sphere(64, 64);
Cube cube;

// light
Light light(glm::vec3(1.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), LIGHT_POINT);

// material
glm::vec3 color(0.101f, 0.544f, 0.318f);
float f0 = 0.596;
glm::vec3 fresnel(0.124 * f0, 0.0498 * f0, 0.0167 * f0);
float metal = 1.0f;
float rough = 0.8f;

Material material(color, rough, fresnel, metal);
Material material_test(color, rough, fresnel, metal);
Material material_predict(color, rough, fresnel, metal);

#ifdef REALTIME_RENDER
int main(int argc, char* argv[])
{
    srand((unsigned int)time(0));

    GLFWwindow* window = initGL();
    //camera.SetRandomPosition();
    
    // build and compile our shader zprogram
    // ------------------------------------
    Shader cookShader("./shader_code/cook_shader.vert", "./shader_code/cook_shader.frag");
    Shader pbrShader("./shader_code/pbr.vert", "./shader_code/pbr.frag");
    Cubemap cubemap("./shader_code/cubemap.vert", "./shader_code/cubemap.frag");
    Irradiancemap irradiancemap("./shader_code/cubemap.vert", "./shader_code/irradiance.frag", &cubemap);
    Prefilteredmap prefiltermap("./shader_code/cubemap.vert", "./shader_code/prefilter.frag", &cubemap);
    BRDFmap brdfmap("./shader_code/brdf.vert", "./shader_code/brdf.frag", &cubemap);
    Shader backgroundShader("./shader_code/background.vert", "./shader_code/background.frag");

    cubemap.loadEnvList(env_path, env_list_name);
    cubemap.loadHDRfromList(env_path, 13);
    cubemap.create();
#ifdef ENVIRONMENT
    irradiancemap.create();
    prefiltermap.create();
    brdfmap.create();

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    backgroundShader.setMat4("projection", projection);
#endif
    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
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
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // shader selection
        if (shading_mode % 2 == 0)
            setCookTorranceShader(&cookShader, &light, &camera, &material);
        else
            setPBShader(&pbrShader, &light, &camera, &material);
        
        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiancemap.getID());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltermap.getID());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfmap.getID());

        sphere.render();
#ifdef BACKGROUND        
        // skybox
        backgroundShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        backgroundShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.getID());
        cube.render();
#endif        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
#endif

#ifdef SAMPLE_RENDER
int main(int argc, char* argv[])
{
    srand((unsigned int)time(0));

    GLFWwindow* window = initGL();

    // build and compile our shader zprogram
    // ------------------------------------
    Shader pbrShader("./shader_code/pbr.vert", "./shader_code/pbr.frag");
    Shader cookShader("./shader_code/cook_shader.vert", "./shader_code/cook_shader.frag");
    Cubemap cubemap("./shader_code/cubemap.vert", "./shader_code/cubemap.frag");
    Irradiancemap irradiancemap("./shader_code/cubemap.vert", "./shader_code/irradiance.frag", &cubemap);
    Prefilteredmap prefiltermap("./shader_code/cubemap.vert", "./shader_code/prefilter.frag", &cubemap);
    BRDFmap brdfmap("./shader_code/brdf.vert", "./shader_code/brdf.frag", &cubemap);
    Shader backgroundShader("./shader_code/background.vert", "./shader_code/background.frag");

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    backgroundShader.setMat4("projection", projection);

    ParameterSample params;
    std::string readPath = param_path + param_name;
    std::vector<std::array<float, 5>> samples = params.readPBRBinary(readPath.c_str(), 1000);
    // loading env list
    cubemap.loadEnvList(env_path, env_list_name);
    int num = env_end_index - env_start_index;
    // sample size for loop
    int sampleSize = samples.size();
    int sampleBatch = sampleSize / num;
    int cnt = 0;
    for (int j = 0; j < num; ++j)
    {
        int env_index = env_start_index + j;
        cubemap.loadHDRfromList(env_path, env_index);
        cubemap.create();
        irradiancemap.create();
        prefiltermap.create();
        brdfmap.create();

        // then before rendering, configure the viewport to the original framebuffer's screen dimensions
        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        glViewport(0, 0, scrWidth, scrHeight);

        for (int i = 0; i < sampleBatch; i += 1)
        {
            int sampleIndex = sampleBatch * j + i;
            camera.SetRandomPosition();

            // per-frame time logic
                // --------------------
            double currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // render
            // ------
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            std::array<float, 5> param = samples[sampleIndex];
            material.setColor(glm::vec3(param[0], param[1], param[2]));
            material.setMetallic(param[3]);
            material.setRoughness(param[4]);
            pbrShader.use();
            setPBShader(&pbrShader, &light, &camera, &material);

            std::string path = img_path + folder_name + "/pbr_";
            // bind pre-computed IBL data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradiancemap.getID());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltermap.getID());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, brdfmap.getID());

            sphere.render();
#ifdef ENABLE_SCREENSHOT
            std::string number = std::to_string(cnt++);
            std::stringstream ss;
            ss << std::setw(5) << std::setfill('0') << number;
            path = path + ss.str() + ".jpg";
            saveScreenshot(path, SCR_WIDTH / 5, SCR_HEIGHT / 5);
#endif
            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
#endif

#ifdef ERROR_RENDER
int main(int argc, char* argv[])
{
    srand((unsigned int)time(0));

    GLFWwindow* window = initGL();

    // build and compile our shader zprogram
    // ------------------------------------
    //Shader phongShader("./shader_code/phong_shader.vert", "./shader_code/phong_shader.frag");
    Shader cookShader("./shader_code/cook_shader.vert", "./shader_code/cook_error.frag");
    Shader pbrShader("./shader_code/pbr.vert", "./shader_code/pbr_error.frag");
    Cubemap cubemap("./shader_code/cubemap.vert", "./shader_code/cubemap.frag");
    Irradiancemap irradiancemap("./shader_code/cubemap.vert", "./shader_code/irradiance.frag", &cubemap);
    Prefilteredmap prefiltermap("./shader_code/cubemap.vert", "./shader_code/prefilter.frag", &cubemap);
    BRDFmap brdfmap("./shader_code/brdf.vert", "./shader_code/brdf.frag", &cubemap);
    Shader backgroundShader("./shader_code/background.vert", "./shader_code/background.frag");

    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);

    cookShader.use();
    cookShader.setInt("irradianceMap", 0);
    cookShader.setInt("prefilterMap", 1);
    cookShader.setInt("brdfLUT", 2);

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    backgroundShader.setMat4("projection", projection);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    ParameterSample params;
    std::string testPath = img_path + folder_name + "/test.bin";
    std::string predictPath = img_path + folder_name + "/predict.bin";
    std::vector<std::array<float, 8>> samples_test = params.loadParams(testPath.c_str(), 2760);
    std::vector<std::array<float, 8>> samples_predict = params.loadParams(predictPath.c_str(), 2760);
    // loading env list
    int num = 0;
    std::vector<std::string> list;
    std::string env_list_path = env_path + env_list_name;
    list = loadEnvList(env_list_path, num);
    num = env_end_index - env_start_index;
    // sample size for loop
    int sampleSize = samples_test.size();
    int sampleBatch = sampleSize / num;
    for (int j = 0; j < num; ++j)
    {
        int env_index = env_start_index + j;
        std::string env_name = env_path + list[env_index] + "/" + list[env_index] + ".hdr";
        cubemap.loadHDR(env_name.c_str());
        cubemap.create();
        irradiancemap.create();
        prefiltermap.create();
        brdfmap.create();

        // then before rendering, configure the viewport to the original framebuffer's screen dimensions
        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        glViewport(0, 0, scrWidth, scrHeight);

        for (int i = 0; i < sampleBatch; ++i)
        {
            int sampleIndex = sampleBatch * j + i;
            //camera.SetRandomPosition();

            // per-frame time logic
                // --------------------
            double currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // render
            // ------
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            std::array<float, 8> param_test = samples_test[sampleIndex];
            std::array<float, 8> param_predict = samples_predict[sampleIndex];
            material_test.setColor(glm::vec3(param_test[0], param_test[1], param_test[2]));
            material_test.setFresnel(glm::vec3(param_test[3] * param_test[6], param_test[4] * param_test[6], param_test[5] * param_test[6]));
            material_test.setRoughness(param_test[7]);
            material_predict.setColor(glm::vec3(param_predict[0], param_predict[1], param_predict[2]));
            material_predict.setFresnel(glm::vec3(param_predict[3] * param_predict[6], param_predict[4] * param_predict[6], param_predict[5] * param_predict[6]));
            material_predict.setRoughness(param_predict[7]);
            cookShader.use();
            setCookErrorShader(&cookShader, &light, &camera, &material_test, &material_predict);

            std::string path = img_path + folder_name + "/error/error_";
            // bind pre-computed IBL data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradiancemap.getID());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltermap.getID());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, brdfmap.getID());

            sphere.render();
#ifdef ENABLE_SCREENSHOT
            std::string number = std::to_string(sampleIndex);
            std::stringstream ss;
            ss << std::setw(5) << std::setfill('0') << number;
            path = path + ss.str() + ".jpg";
            saveScreenshot(path, SCR_WIDTH / 5, SCR_HEIGHT / 5);
#endif
            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
#endif

#ifdef SIMPLE_MAIN
int main()
{
    ParameterSample params;
    //std::vector<std::array<float, 7>> samples = cookParams.readCookParams("../../img/cook/cook_c10_d10_r50_params.bin");
    //params.makeCookParams(cook_path, 40000, 1);
    params.makePBRParams(pbr5_path, 10000, 1);
    //std::vector<std::array<float, 5>> samples = params.readPBRParams("../../img/pbr5/pbr_c200_r25_params.bin");
    
    return 0;
}
#endif

GLFWwindow* initGL()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Renderer", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return window;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return window;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    return window;
}

void setCookTorranceShader(Shader* pShader, Light* pLight, Camera* pCamera, Material* pMaterial)
{
    pShader->use();
    pShader->setInt("irradianceMap", 0);
    pShader->setInt("prefilterMap", 1);
    pShader->setInt("brdfLUT", 2);

    // pass projection, view, and model matrices to shader
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    pShader->setMat4("projection", projection);
    glm::mat4 view = camera.GetViewMatrix();
    pShader->setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    pShader->setMat4("model", model);

    // light
    pShader->setVec3("lightPos", light.getPosition());
    pShader->setVec3("lightColor", light.getColor());
    pShader->setInt("lightType", light.getType());

    // camera uniform variable
    pShader->setVec3("viewPos", camera.getPosition());

    pShader->setVec3("albedo", pMaterial->getColor());
    pShader->setFloat("roughness", pMaterial->getRoughness());
    pShader->setVec3("fresnel", pMaterial->getFresnel());
}

void setPBShader(Shader* pShader, Light* pLight, Camera* pCamera, Material* pMaterial)
{
    pShader->use();
    pShader->setInt("irradianceMap", 0);
    pShader->setInt("prefilterMap", 1);
    pShader->setInt("brdfLUT", 2);

    // pass projection, view, and model matrices to shader
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    pShader->setMat4("projection", projection);
    glm::mat4 view = camera.GetViewMatrix();
    pShader->setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    pShader->setMat4("model", model);

    // light
    pShader->setVec3("lightPos", light.getPosition());
    pShader->setVec3("lightColor", light.getColor());
    pShader->setInt("lightType", light.getType());

    // camera uniform variable
    pShader->setVec3("camPos", camera.getPosition());

    pShader->setVec3("albedo", pMaterial->getColor());
    pShader->setFloat("roughness", pMaterial->getRoughness());
    pShader->setFloat("metallic", pMaterial->getMetallic());
}

void setCookErrorShader(Shader* pShader, Light* pLight, Camera* pCamera, Material* pMaterial1, Material* pMaterial2)
{
    pShader->use();
    pShader->setInt("irradianceMap", 0);
    pShader->setInt("prefilterMap", 1);
    pShader->setInt("brdfLUT", 2);

    // pass projection, view, and model matrices to shader
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    pShader->setMat4("projection", projection);
    glm::mat4 view = camera.GetViewMatrix();
    pShader->setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    pShader->setMat4("model", model);

    // light
    pShader->setVec3("lightPos", light.getPosition());
    pShader->setVec3("lightColor", light.getColor());
    pShader->setInt("lightType", light.getType());

    // camera uniform variable
    pShader->setVec3("viewPos", camera.getPosition());

    pShader->setVec3("albedo1", pMaterial1->getColor());
    pShader->setFloat("roughness1", pMaterial1->getRoughness());
    pShader->setVec3("fresnel1", pMaterial1->getFresnel());
    pShader->setVec3("albedo2", pMaterial2->getColor());
    pShader->setFloat("roughness2", pMaterial2->getRoughness());
    pShader->setVec3("fresnel2", pMaterial2->getFresnel());
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
        saveScreenshot("screen00.jpg", SCR_WIDTH, SCR_HEIGHT);
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        shading_mode += 1;
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

bool saveHDRScreenshot(std::string filename, int width, int height)
{
    // row alignment
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    //int nScrSize = SCR_WIDTH * SCR_HEIGHT * 4;
    int nSize = width * height * 4;
    float* dataBuffer = (float*)malloc(nSize * sizeof(float));
    if (!dataBuffer) {
        std::cout << "saveScreenshot() :: buffer allocation error." << std::endl;
        return false;
    }

    // fetch image from the backbuffer
    glReadPixels((GLint)0, (GLint)0, (GLint)width, (GLint)height, GL_RGBA, GL_FLOAT, dataBuffer);

    stbi_flip_vertically_on_write(true);
    stbi_write_hdr(filename.c_str(), width, height, 4, dataBuffer);

    free(dataBuffer);

    std::cout << "saving HDR screenshot(" << filename << ")\n";
    return true;
}

std::vector<std::string> loadEnvList(std::string filename, int& num)
{
    std::ifstream fin;

    fin.open(filename, std::ios::in);
    std::string line;   // buffer
    int cnt = 0;        // size counting

    std::vector<std::string> list;      // list of filenames

    if (fin.is_open())
    {
        while (std::getline(fin, line))
        {
            //std::cout << line << std::endl;
            list.push_back(line);
            cnt++;
        }
        num = cnt;
        fin.close();
    }
    else
    {
        std::cout << "(loadEnvList) unable to open " << filename << std::endl;
    }
    return list;
}