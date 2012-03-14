#version 130

out vec4 out_colour;
in vec2 texCoord;

uniform vec2 pixSize;
uniform sampler2D baseTex;

void main()
{	
	out_colour = texture(baseTex, texCoord);
}

