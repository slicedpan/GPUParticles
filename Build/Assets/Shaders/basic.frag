#version 130

out vec4 out_colour;

uniform vec3 lightDir = vec3(0-1, -1, 0);

smooth in vec3 out_normal;

void main()
{	
	float nDotL = max(dot(normalize(out_normal), normalize(-lightDir)), 0.0);
	out_colour = vec4(1.0, 1.0, 1.0, 1.0) * nDotL;
}

