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
uniform vec3 cookColor;
uniform vec3 cookFresnel;
uniform float cookRoughness;

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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	vec3 N = normalize(Normal);
	vec3 L = normalize(lightPos - FragPos);
	vec3 V = normalize(viewPos - FragPos);
	vec3 H = normalize(L + V);
	vec3 R = reflect(-V, N);

	float distance = length(lightPos - FragPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = lightColor * attenuation;

	float NdotL = max(dot(N, L), 0.0);

	vec3 result = vec3(0.0);

	float NDF = DistributionGGX(N, H, cookRoughness);
	float G = GeometrySmith(N, V, L, cookRoughness);
	vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), cookFresnel);

	vec3 Ks = F;
	vec3 Kd = vec3(1.0) - Ks;

	vec3 numerator = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);

	result = (Kd * cookColor / 3.141592 + specular) * radiance * NdotL;

	// ambient lighting (we now use IBL as the ambient term)
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * cookColor;
    
	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  cookRoughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), cookRoughness)).rg;
    specular = prefilteredColor;// * (F * brdf.x + brdf.y);
	
	vec3 ambient = Kd * diffuse + specular;

	vec3 color = ambient;// + result;
	
	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// gamma
	color = pow(color, vec3(1.0/2.2));

	//FragColor = mix(vec4(result, 1.0f), texture(texture1, TexCoord), 0.0);
	FragColor = vec4(color, 1.0f);
}