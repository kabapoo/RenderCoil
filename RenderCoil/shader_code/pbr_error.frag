#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform vec3 albedo1;
uniform float metallic1;
uniform float roughness1;

// material parameters
uniform vec3 albedo2;
uniform float metallic2;
uniform float roughness2;

uniform float ao;


// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 camPos;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------
void main()
{		
    vec3 N = Normal;
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0_1 = vec3(0.04); 
    vec3 F0_2 = vec3(0.04);
    F0_1 = mix(F0_1, albedo1, metallic1);
    F0_2 = mix(F0_2, albedo2, metallic2);
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F_1 = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0_1, roughness1);
    vec3 F_2 = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0_2, roughness2);
    
    vec3 kS_1 = F_1;
    vec3 kD_1 = 1.0 - kS_1;
    kD_1 *= 1.0 - metallic1;

    vec3 kS_2 = F_2;
    vec3 kD_2 = 1.0 - kS_2;
    kD_2 *= 1.0 - metallic2;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse1   = irradiance * albedo1;
    vec3 diffuse2   = irradiance * albedo2;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor1 = textureLod(prefilterMap, R,  roughness1 * MAX_REFLECTION_LOD).rgb;
    vec3 prefilteredColor2 = textureLod(prefilterMap, R,  roughness2 * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf1  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness1)).rg;
    vec2 brdf2  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness2)).rg;
    vec3 specular1 = prefilteredColor1 * (F_1 * brdf1.x + brdf1.y);
    vec3 specular2 = prefilteredColor2 * (F_2 * brdf2.x + brdf2.y);

    vec3 ambient1 = (kD_1 * diffuse1 + specular1) * ao;
    vec3 ambient2 = (kD_2 * diffuse2 + specular2) * ao;
    
    vec3 color = sqrt((ambient1 - ambient2) * (ambient1 - ambient2));

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color , 1.0);
}