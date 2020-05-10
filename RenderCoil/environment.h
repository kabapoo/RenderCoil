#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <iostream>

#include "shader.h"
#include "polygon.h"

class Cubemap
{
private:
    unsigned int id;
    unsigned int hdr;
    unsigned int fbo;
    unsigned int rbo;
    glm::mat4 projection;
    glm::mat4 views[6];
    
    Cube cube;

public:
    Cubemap();
    Cubemap(const char* fname);
    unsigned int GetID() const { return id; }

    void setupFramebuffer();
    void loadHDR(const char* fname);
    void setupCubemap();
    void setupMatrices();
    void render();

    unsigned int getID() const { return id; }
    unsigned int getFBO() const { return fbo; }
    unsigned int getRBO() const { return rbo; }
    glm::mat4 getProjection() const { return projection; }
    glm::mat4 getViews(int i) const { return views[i]; }

    Shader* pShader;
};

class Irradiancemap
{
private:
    unsigned int id;
    Cubemap* pCubemap;
    
    Cube cube;

public:
    Irradiancemap(Cubemap* p);

    void create();
    Shader* pShader;
};