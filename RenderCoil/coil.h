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
#include "light.h"
#include "environment.h"
#include "sample_io.h"

//#define REALTIME_RENDER
#define SAMPLE_RENDER
//#define MAKE_LABEL
//#define ERROR_RENDER

#define COOK
//#define PBR

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
bool saveScreenshot(std::string filename, int width, int height);
bool saveHDRScreenshot(std::string filename, int width, int height);
void setCommonUniforms(Shader* pShader);
std::vector<std::string> loadEnvList(std::string filename, int& num);

std::string env_path = "../../img/envs/";