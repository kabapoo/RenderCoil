#include "coil.h"

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 640;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
double lastX = SCR_WIDTH / 2.0;
double lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// sphere initialzied as radius, sectors, stacks
Sphere sphere(64, 64);
Cube cube;

// light
Light light(glm::vec3(0.8f, 1.5f, 2.3f), glm::vec3(1.0f, 1.0f, 1.0f), LIGHT_POINT);

// timing
double deltaTime = 0.0;	// time between current frame and last frame
double lastFrame = 0.0;

#ifdef REALTIME_RENDER
int main(int argc, char* argv[])
{
    srand((unsigned int)time(0));
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
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

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
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader cookShader("./shader_code/cook_shader.vert", "./shader_code/cook_shader.frag");
    Shader pbrShader("./shader_code/pbr.vert", "./shader_code/pbr.frag");
    Cubemap cubemap("./shader_code/cubemap.vert", "./shader_code/cubemap.frag");
    Irradiancemap irradiancemap("./shader_code/cubemap.vert", "./shader_code/irradiance.frag", &cubemap);
    Prefilteredmap prefiltermap("./shader_code/cubemap.vert", "./shader_code/prefilter.frag", &cubemap);
    BRDFmap brdfmap("./shader_code/brdf.vert", "./shader_code/brdf.frag", &cubemap);
    Shader backgroundShader("./shader_code/background.vert", "./shader_code/background.frag");
#ifdef PBR
    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);
#endif
#ifdef COOK
    cookShader.use();
    cookShader.setInt("irradianceMap", 0);
    cookShader.setInt("prefilterMap", 1);
    cookShader.setInt("brdfLUT", 2);
#endif
    int num = 0;
    std::vector<std::string> list;
    std::string env_list_path = env_path + "env_list.txt";
    list = loadEnvList(env_list_path, num);
    std::string env_name = env_path + list[0] + "/" + list[0] + "_Ref.hdr";
    std::cout << env_name << std::endl;
    cubemap.loadHDR(env_name.c_str());
    cubemap.create();
    irradiancemap.create();
    prefiltermap.create();
    brdfmap.create();

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    backgroundShader.setMat4("projection", projection);

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

        glm::mat4 view = camera.GetViewMatrix();

        // pbr config
#ifdef PBR
        pbrShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        pbrShader.setMat4("projection", projection);
        pbrShader.setMat4("model", model);
        pbrShader.setMat4("view", view);
        pbrShader.setVec3("camPos", camera.Position);
        pbrShader.setFloat("ao", 1.0f);
        pbrShader.setVec3("albedo", 0.146f, 0.504f, 0.862f);
        pbrShader.setFloat("metallic", 0.124f);
        pbrShader.setFloat("roughness", 0.474f);
#endif
#ifdef COOK
        cookShader.use();
        setCommonUniforms(&cookShader);
        cookShader.setVec3("cookColor", glm::vec3(0.5f, 0.3f, 0.8f));
        cookShader.setFloat("cookRoughness", 0.02f);
        cookShader.setVec3("cookFresnel", glm::vec3(0.9f, 0.4f, 0.4f));
#endif
        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiancemap.getID());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltermap.getID());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfmap.getID());

        sphere.render();
        
        // skybox
        backgroundShader.use();
        backgroundShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.getID());
        cube.render();
        
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
        return -1;
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
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader pbrShader("./shader_code/pbr.vert", "./shader_code/pbr.frag");
    Shader cookShader("./shader_code/cook_shader.vert", "./shader_code/cook_shader.frag");
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

    ParameterSample params;
    int c = 10000;
    int d = 0;
    int r = 1;
#ifdef PBR
    std::string name = "pbr_c" + std::to_string(c);
    if (d != 0) name = name + "_d" + std::to_string(d);
    name = name + "_r" + std::to_string(r);
    std::string readPath = "../../img/pbr5/" + name + "_params.bin";
    std::vector<std::array<float, 5>> samples = params.readPBRParams(readPath.c_str());
#endif
#ifdef COOK
    std::string name = "cook_c" + std::to_string(c);
    if (d != 0) name = name + "_d" + std::to_string(d);
    name = name + "_r" + std::to_string(r);
    std::string readPath = "../../img/cook/" + name + "_params.bin";
    std::vector<std::array<float, 7>> samples = params.readCookParams(readPath.c_str());
#endif
    // loading env list
    int num = 0;
    std::vector<std::string> list;
    std::string env_list_path = env_path + "env_list.txt";
    list = loadEnvList(env_list_path, num);
    // sample size for loop
    int sampleSize = samples.size();
    int sampleBatch = sampleSize / 10;
    for (int j = 0; j < num; ++j)
    {
        std::string env_name = env_path + list[j] + "/" + list[j] + "_Ref.hdr";
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
            // per-frame time logic
                // --------------------
            double currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // render
            // ------
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef PBR
            std::array<float, 5> param = samples[i];
            pbrShader.use();
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model = glm::mat4(1.0f);
            pbrShader.setMat4("projection", projection);
            pbrShader.setMat4("model", model);
            pbrShader.setMat4("view", view);
            pbrShader.setVec3("camPos", camera.Position);
            pbrShader.setVec3("albedo", param[0], param[1], param[2]);
            pbrShader.setFloat("ao", 1.0f);
            pbrShader.setFloat("metallic", param[3]);
            pbrShader.setFloat("roughness", param[4]);

            std::string path = "../../img/pbr5/";
            path = path + name + "/pbr";
#endif
#ifdef COOK
            std::array<float, 7> param = samples[sampleIndex];
            cookShader.use();
            setCommonUniforms(&cookShader);
            cookShader.setVec3("cookColor", glm::vec3(param[0], param[1], param[2]));
            cookShader.setFloat("cookRoughness", param[3]);
            cookShader.setVec3("cookFresnel", glm::vec3(param[4], param[5], param[6]));

            std::string path = "../../img/cook/";
            path = path + name + "/cook";
#endif
            // bind pre-computed IBL data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradiancemap.getID());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltermap.getID());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, brdfmap.getID());

            sphere.render();
            
            std::string number = std::to_string(sampleIndex);
            std::stringstream ss;
            ss << std::setw(5) << std::setfill('0') << number;
            path = path + ss.str() + ".jpg";
            saveScreenshot(path, SCR_WIDTH / 10, SCR_HEIGHT / 10);
            
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
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

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
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // build and compile our shader zprogram
    // ------------------------------------
    //Shader phongShader("./shader_code/phong_shader.vert", "./shader_code/phong_shader.frag");
    Shader cookShader("./shader_code/cook_shader.vert", "./shader_code/cook_error.frag");
    Shader pbrShader("./shader_code/pbr.vert", "./shader_code/pbr_error.frag");
    Cubemap cubemap("./shader_code/cubemap.vert", "./shader_code/cubemap.frag", "../../img/envs/Newport_Loft/Newport_Loft_Ref.hdr");
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
    int c = 10000;
    int d = 0;
    int r = 1;
    int img_num = 3000;
#ifdef COOK
    std::string _type = "cook";
#endif
#ifdef PBR
    std::string _type = "pbr5";
#endif
    std::string name = _type + "_c" + std::to_string(c);
    if (d != 0) name = name + "_d" + std::to_string(d);
    name = name + "_r" + std::to_string(r);
    std::string testPath = "../../img/" + _type + "/params/" + name + "_test.bin";
    std::string predictPath = "../../img/" + _type + "/params/" + name + "_predict.bin";
#ifdef COOK
    std::vector<std::array<float, 7>> tests = params.readCookBinary(testPath.c_str(), img_num);
    std::vector<std::array<float, 7>> predicts = params.readCookBinary(predictPath.c_str(), img_num);
#endif
#ifdef PBR
    std::vector<std::array<float, 5>> tests = params.readPBRBinary(testPath.c_str(), img_num);
    std::vector<std::array<float, 5>> predicts = params.readPBRBinary(predictPath.c_str(), img_num);
#endif
    if (tests.size() != predicts.size())
    {
        std::cout << "test and predict size not match\n";
        return 0;
    }

    // render loop
    // -----------
    for (int i = 0; i < tests.size(); ++i)
    {
        // per-frame time logic
        // --------------------
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        //processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        
        // pbr config
#ifdef PBR
        std::array<float, 5> test_params = tests[i];
        std::array<float, 5> predict_params = predicts[i];

        pbrShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        pbrShader.setMat4("projection", projection);
        pbrShader.setMat4("model", model);
        pbrShader.setMat4("view", view);
        pbrShader.setVec3("camPos", camera.Position);
        pbrShader.setFloat("ao", 1.0f);
        pbrShader.setVec3("albedo1", test_params[0], test_params[1], test_params[2]);
        pbrShader.setFloat("metallic1", test_params[3]);
        pbrShader.setFloat("roughness1", test_params[4]);
        pbrShader.setVec3("albedo2", predict_params[0], predict_params[1], predict_params[2]);
        pbrShader.setFloat("metallic2", predict_params[3]);
        pbrShader.setFloat("roughness2", predict_params[4]);
#endif
#ifdef COOK
        std::array<float, 7> test_params = tests[i];
        std::array<float, 7> predict_params = predicts[i];

        cookShader.use();
        setCommonUniforms(&cookShader);
        cookShader.setVec3("cookColor1", test_params[0], test_params[1], test_params[2]);
        cookShader.setFloat("cookRoughness1", test_params[3]);
        cookShader.setVec3("cookFresnel1", test_params[4], test_params[5], test_params[6]);
        cookShader.setVec3("cookColor2", predict_params[0], predict_params[1], predict_params[2]);
        cookShader.setFloat("cookRoughness2", predict_params[3]);
        cookShader.setVec3("cookFresnel2", predict_params[4], predict_params[5], predict_params[6]);
#endif
        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiancemap.getID());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltermap.getID());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfmap.getID());

        sphere.render();

        std::string path = "../../img/" + _type + "/error/";
        path = path + name + "/error";
        std::string number = std::to_string(i);
        std::stringstream ss;
        ss << std::setw(5) << std::setfill('0') << number;
        path = path + ss.str() + ".jpg";
        saveScreenshot(path, SCR_WIDTH / 5, SCR_HEIGHT / 5);

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


#ifdef MAKE_LABEL
int main()
{
    ParameterSample params;
    //std::vector<std::array<float, 7>> samples = cookParams.readCookParams("../../img/cook/cook_c10_d10_r50_params.bin");
    //params.makeCookParams("../../img/cook/", 400, 25);
    //params.makePBRParams("../../img/pbr5/", 400, 25);
    //std::vector<std::array<float, 5>> samples = params.readPBRParams("../../img/pbr5/pbr_c200_r25_params.bin");
    
    return 0;
}
#endif


void setCommonUniforms(Shader* pShader)
{
    // pass projection matrix to shader (note that in this case it could change every frame)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    pShader->setMat4("projection", projection);

    // camera/view transformation
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
        saveHDRScreenshot("screen00.hdr", SCR_WIDTH, SCR_HEIGHT);
    //if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        //switchCubemap();
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