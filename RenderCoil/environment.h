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
    //unsigned int irradi_id;
    //unsigned int mip_id;
    //unsigned int brdf_id;
    unsigned int hdr;
    unsigned int fbo;
    unsigned int rbo;
    glm::mat4 projection;
    glm::mat4 views[6];
    
    Cube cube;
    Quad quad;

public:
    Cubemap();
    Cubemap(const char* vert, const char* frag);

    void loadHDR(const char* fname);
    void setupMatrices();
    void create();
    void createIrradianceMap();

    unsigned int getID() const { return id; }
    unsigned int getHDR() const { return hdr; }
    unsigned int getFBO() const { return fbo; }
    unsigned int getRBO() const { return rbo; }
    glm::mat4 getProjection() const { return projection; }
    glm::mat4 getViews(int i) const { return views[i]; }

    Shader* pShader;
    //Shader* pIrradiShader;
    //Shader* pMipShader;
    //Shader* pBRDFShader;
};

class Irradiancemap
{
private:
    unsigned int id;
    Cubemap* pCubemap;
    Cube cube;

public:
    Irradiancemap(const char* vert, const char* frag, Cubemap* p);
    unsigned int getID() const { return id; }
    void create();
    Shader* pShader;
};

class Prefilteredmap
{
private:
    unsigned int id;
    Cubemap* pCubemap;
    Cube cube;

public:
    Prefilteredmap(const char* vert, const char* frag, Cubemap* p);
    unsigned int getID() const { return id; }
    void create();
    Shader* pShader;
};

class BRDFmap
{
private:
    unsigned int id;
    Cubemap* pCubemap;
    Quad quad;

public:
    BRDFmap(const char* vert, const char* frag, Cubemap* p);
    unsigned int getID() const { return id; }
    void create();
    Shader* pShader;
};
