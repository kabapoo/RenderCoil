#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Material
{
protected:
    glm::vec3 mColor;
    float mRoughness;
    glm::vec3 mFresnel;
    float mMetallic;
    
public:
    Material(glm::vec3 color, float rough, glm::vec3 fresnel, float metal)
    {
        mColor = color;
        mRoughness = rough;
        mFresnel = fresnel;
        mMetallic = metal;
    }

    void setColor(glm::vec3 color);
    glm::vec3 getColor() const { return mColor; }
    glm::vec3 getFresnel() const { return mFresnel; }
    float getRoughness() const { return mRoughness; }
    float getMetallic() const { return mMetallic; }
};

#endif
