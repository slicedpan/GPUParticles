#version 130

out vec4 out_colour;

uniform vec3 lightPos = vec3(0, 10, 0);
uniform float lightRadius = 20.0;

smooth in vec3 out_normal;
smooth in vec3 worldPos;

void main()
{	
	vec3 lightDir = lightPos - worldPos;
	float lightDist = length(lightDir);
	lightDir /= lightDist;
	float nDotL = max(dot(normalize(out_normal), lightDir), 0.0);
	float attenuation = pow(max(1 - lightDist / lightRadius, 0.0), 2.0);
	out_colour = vec4(1.0, 1.0, 1.0, 1.0) * nDotL * attenuation;
}

