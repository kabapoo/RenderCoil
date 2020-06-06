#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// light
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform int lightType;
// view
uniform vec3 viewPos;
// material
uniform vec3 cookColor1;
uniform vec3 cookFresnel1;
uniform float cookRoughness1;
uniform vec3 cookColor2;
uniform vec3 cookFresnel2;
uniform float cookRoughness2;


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nominator = a2;
	float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
	denominator = 3.141592 * denominator * denominator;

	return nominator / max(denominator, 0.00000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float theta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - theta, 5.0);
}

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	vec3 N = normalize(Normal);
	vec3 L = normalize(lightPos - FragPos);
	vec3 V = normalize(viewPos - FragPos);
	vec3 H = normalize(L + V);
	vec3 R = reflect(-V, N);

	vec3 F1 = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), cookFresnel1);
	vec3 F2 = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), cookFresnel2);

	vec3 Ks1 = F1;
	vec3 Kd1 = vec3(1.0) - Ks1;
	vec3 Ks2 = F2;
	vec3 Kd2 = vec3(1.0) - Ks2;

	// ambient lighting (we now use IBL as the ambient term)
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse1   = irradiance * cookColor1;
	vec3 diffuse2   = irradiance * cookColor2;
    
	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor1 = textureLod(prefilterMap, R,  cookRoughness1 * MAX_REFLECTION_LOD).rgb;    
	vec3 prefilteredColor2 = textureLod(prefilterMap, R,  cookRoughness2 * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf1  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), cookRoughness1)).rg;
	vec2 brdf2  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), cookRoughness2)).rg;
    vec3 specular1 = prefilteredColor1 * (F1 * brdf1.x + brdf1.y);
	vec3 specular2 = prefilteredColor2 * (F2 * brdf2.x + brdf2.y);
	
	vec3 ambient1 = Kd1 * diffuse1 + specular1;
	vec3 ambient2 = Kd2 * diffuse2 + specular2;

	vec3 color = sqrt((ambient1 - ambient2) * (ambient1 - ambient2)) * 1.0f;
	
	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// gamma
	color = pow(color, vec3(1.0/2.2));

	//FragColor = mix(vec4(result, 1.0f), texture(texture1, TexCoord), 0.0);
	FragColor = vec4(color, 1.0f);
}