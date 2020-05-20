#include "environment.h"

Cubemap::Cubemap()
{
    id = 0;
    hdr = 0;
    fbo = 0;
    rbo = 0;
    pShader = nullptr;
}

Cubemap::Cubemap(const char* vert, const char* frag, const char* fname)
{
    pShader = new Shader(vert, frag);
    setupFramebuffer();
    loadHDR(fname);
    setupCubemap();
    setupMatrices();
    render();
}

void Cubemap::setupFramebuffer()
{
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void Cubemap::loadHDR(const char* fname)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(fname, &width, &height, &nrComponents, 0);

    if (data)
    {
        glGenTextures(1, &hdr);
        glBindTexture(GL_TEXTURE_2D, hdr);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << fname << " loaded" << std::endl;
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }
}

// setup cubemap to render to and attach to framebuffer
void Cubemap::setupCubemap()
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// set up projection and view matrices for capturing data onto the 6 cubemap face directions
void Cubemap::setupMatrices()
{
    projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    views[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f));
    views[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f));
    views[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    views[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f));
    views[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f));
    views[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3(0.0f,-1.0f, 0.0f));
}

// convert HDR equirectangular environment map to cubemap equivalent
void Cubemap::render()
{
    pShader->use();
    pShader->setInt("equirectangularMap", 0);
    pShader->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr);
    
    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (unsigned int i = 0; i < 6; i++)
    {
        pShader->setMat4("view", views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube.render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


Irradiancemap::Irradiancemap(const char* vert, const char* frag, Cubemap* p)
{
    pShader = new Shader(vert, frag);
    pCubemap = p;
    create();
}

// create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
void Irradiancemap::create()
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    glBindRenderbuffer(GL_RENDERBUFFER, pCubemap->getRBO());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    pShader->use();
    pShader->setInt("environmentMap", 0);
    pShader->setMat4("projection", pCubemap->getProjection());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pCubemap->getID());

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    for (unsigned int i = 0; i < 6; ++i)
    {
        pShader->setMat4("view", pCubemap->getViews(i));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube.render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Prefilteredmap::Prefilteredmap(const char* vert, const char* frag, Cubemap* p)
{
    pShader = new Shader(vert, frag);
    pCubemap = p;
    create();
}

// create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
void Prefilteredmap::create()
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    pShader->use();
    pShader->setInt("environmentMap", 0);
    pShader->setMat4("projection", pCubemap->getProjection());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pCubemap->getID());

    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, pCubemap->getRBO());
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        pShader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            pShader->setMat4("view", pCubemap->getViews(i));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube.render();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

BRDFmap::BRDFmap(const char* vert, const char* frag, Cubemap* p)
{
    pShader = new Shader(vert, frag);
    pCubemap = p;
    create();
}

// create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
void BRDFmap::create()
{
    glGenTextures(1, &id);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    glBindRenderbuffer(GL_RENDERBUFFER, pCubemap->getRBO());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);

    glViewport(0, 0, 512, 512);
    pShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quad.render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}