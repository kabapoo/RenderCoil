#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <array>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"
#include "shader.h"
#include "camera.h"
#include "polygon.h"
#include "material.h"
#include "light.h"
#include "environment.h"
#include "sample_io.h"

#define REALTIME_RENDER
//#define SAMPLE_RENDER
//#define SIMPLE_MAIN
//#define ERROR_RENDER
#define ENABLE_SCREENSHOT
#define ENVIRONMENT
#define BACKGROUND

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

GLFWwindow* initGL();
bool saveScreenshot(std::string filename, int width, int height);
bool saveHDRScreenshot(std::string filename, int width, int height);
void setCookTorranceShader(Shader* pShader, Light* pLight, Camera* pCamera, Material* pMaterial);
void setPBShader(Shader* pShader, Light* pLight, Camera* pCamera, Material* pMaterial);
void setCookErrorShader(Shader* pShader, Light* pLight, Camera* pCamera, Material* pMaterial1, Material* pMaterial2);
std::vector<std::string> loadEnvList(std::string filename, int& num);

std::string env_path = "../../img/envs/";
std::string img_path = "../../img/";
std::string param_path = "../../Tensorflow/params/";
std::string param_name = "pbr_test.bin";
std::string folder_name = "pbr_test";
std::string env_list_name = "env_list.txt";

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 640;
double lastX = SCR_WIDTH / 2.0;
double lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;
int shading_mode = 1;

// timing
double deltaTime = 0.0;	// time between current frame and last frame
double lastFrame = 0.0;

const int env_start_index = 10;
const int env_end_index = 20;