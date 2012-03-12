#version 130

out vec4 out_colour;
in vec3 colour;

void main()
{	
	out_colour = vec4(colour, 1.0);
}

