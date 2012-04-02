#version 330

layout(location = 0)out vec4 out_colour;

smooth in vec3 out_normal;
smooth in float depth;

void main()
{		
	out_colour = vec4(out_normal, depth);
}

