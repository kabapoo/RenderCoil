#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// light
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform int lightType;
// view
uniform vec3 viewPos;
// material
uniform vec3 phongColor;
uniform float phongGlossiness;
uniform float phongSpecularPower;

float BlinnPhong(vec3 N, vec3 H, float g)
{
	float spec = pow(max(dot(N, H), 0.0), g);

	return spec;
}

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	//FragColor = mix(vec4(Normal, 1.0f), texture(texture2, TexCoord), 0.2);
	vec3 N = normalize(Normal);
	vec3 L = normalize(lightPos - FragPos);
	vec3 V = normalize(viewPos - FragPos);
	vec3 H = normalize(L + V);
	vec3 R = reflect(-L, N);

	float distance = length(lightPos - FragPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = lightColor * attenuation;

	float NdotL = max(dot(N, L), 0.0);

	vec3 result = vec3(0.0);

	// select shading model for the specular calculation
	vec3 diff = phongColor * NdotL * radiance;
	//vec3 spec = pow(max(dot(V, R), 0.0), phongGlossiness) * phongSpecularPower * radiance;
	vec3 spec = BlinnPhong(N, H, phongGlossiness) * phongSpecularPower * radiance;
	result = diff + spec;

	// FragColor = mix(vec4(result, 1.0f), texture(texture1, TexCoord), 0.0);

	// HDR tonemapping
	vec3 color = result / (result + vec3(1.0));
	// gamma
	color = pow(color, vec3(1.0/2.2));

	FragColor = vec4(color, 1.0f);
}